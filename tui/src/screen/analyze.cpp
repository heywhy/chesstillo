#include <algorithm>
#include <cstdint>
#include <format>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "engine/move.hpp"
#include "engine/position.hpp"
#include "engine/square.hpp"
#include "engine/types.hpp"
#include "engine/utility.hpp"
#include "uci/command.hpp"
#include "uci/engine.hpp"
#include "uci/types.hpp"
#include "uci/utils.hpp"

#include "tui/color.hpp"
#include "tui/component/chessboard.hpp"
#include "tui/component/engine_settings.hpp"
#include "tui/component/input.hpp"
#include "tui/component/modal_view.hpp"
#include "tui/component/square.hpp"
#include "tui/component/view.hpp"
#include "tui/config.hpp"
#include "tui/constants.hpp"
#include "tui/fonts.hpp"
#include "tui/mapping.hpp"
#include "tui/screen/analyze.hpp"
#include "tui/theme.hpp"
#include "tui/types.hpp"
#include "tui/utils.hpp"

using namespace std::chrono_literals;

namespace tui {
namespace screen {

Analyze::Analyze(const Theme &theme)
    : component::View(tui::Make<analyze::Main>(theme, this, engine_options_)) {}

void Analyze::BindKeymaps() {
  View::BindKeymaps();

  auto *main = dynamic_cast<analyze::Main *>(main_content_.get());

  // TODO: remove piece on backspace in visual mode
  SetKeymap(tui::NORMAL, "R", std::bind(&analyze::Main::Stop, main));
  SetKeymap(tui::NORMAL, "r", std::bind(&analyze::Main::Start, main));
  SetKeymap(tui::NORMAL, "c", std::bind(&analyze::Main::ClearHash, main));
  SetKeymap(tui::NORMAL, "s", std::bind(&analyze::Main::ShowSettings, main));
  SetKeymap(tui::NORMAL, "a", std::bind(&analyze::Main::ShowEngineInfo, main));
}

namespace analyze {
Main::Main(const Theme &theme, component::View *view,
           EngineOptions &engine_options)
    : theme_(theme),
      view_(view),
      engine_options_(engine_options),
      fen_(engine::kStartPos),
      selected_(nullptr),
      engine_(uci::FindExecutable("stockfish"), this),
      thread_(&Main::InitEngine, this) {
  engine::Position::ApplyFen(&position_, fen_);

  Add(MakeContainer());
  UpdateBoard();
}

Main::~Main() { thread_.join(); }

ftxui::Component Main::MakeContainer() {
  fen_input_ = tui::Make<component::Input>(
      "FEN", fen_,
      component::InputOption{.on_enter = std::bind(&Main::OnFenChange, this)});

  pgn_input_ = tui::Make<component::Input>(
      "PGN", pgn_, true,
      component::InputOption{.on_enter = std::bind(&Main::OnPgnChange, this)});

  chessboard_ = tui::Make<component::Chessboard>(
      theme_, std::bind(&Main::MaybeMove, this, std::placeholders::_1));

  return ftxui::Container::Vertical({
      chessboard_,
      fen_input_,
      pgn_input_,
  });
}

void Main::OnFenChange() {
  std::lock_guard lock(mutex_);
  const bool restart = running_;

  Stop();
  engine::Position::ApplyFen(&position_, fen_);
  engine_.Send(uci::command::Input("ucinewgame"));
  UpdateBoard();

  if (restart) {
    Start();
  }
}

void Main::OnPgnChange() {}

void Main::ToggleSquare(component::Square *square) {
  if (view_->Mode() != tui::VISUAL) [[likely]] {
    chessboard_->ToggleSquare(square);
  }
}

void Main::MaybeMove(component::Square *square) {
  if (selected_ == nullptr) {
    ToggleSquare(square);

    selected_ = square;
  } else if (selected_ == square) {
    ToggleSquare(selected_);

    selected_ = nullptr;
  } else if (selected_->Empty() && square->Empty()) {
    ToggleSquare(selected_);
    ToggleSquare(square);

    selected_ = square;
  } else [[likely]] {
    const engine::Move *move = nullptr;
    std::lock_guard lock(mutex_);
    const engine::MoveList move_list = position_.LegalMoves();

    for (const auto &m : move_list) {
      if ((m.from == selected_->index && m.to == square->index) ||
          (m.from == square->index && m.to == selected_->index)) {
        move = &m;
        break;
      }
    }

    if (move) {
      const bool restart = running_;

      Stop();

      position_.Make(*move);
      ToggleSquare(selected_);

      if (square->index != move->to) {
        chessboard_->SetActiveChild(selected_);
      }

      UpdateBoard();

      selected_ = nullptr;
      fen_ = position_.ToFen();

      if (restart) {
        Start();
      }
    } else {
      ToggleSquare(selected_);
      chessboard_->SetActiveChild(selected_);

      selected_ = nullptr;
    }
  }
}

bool Main::OnEvent(ftxui::Event event) {
  if (ftxui::ComponentBase::OnEvent(event)) {
    return true;
  }

  bool handled = false;

  for (auto &pv : pvs_) {
    if (!pv.id) {
      break;
    }

    if (pv.OnEvent(event)) {
      handled = true;
      break;
    }
  }

  return handled;
}

ftxui::Element Main::OnRender() {
  static ftxui::FlexboxConfig config = {
      .direction = ftxui::FlexboxConfig::Direction::Column,
      .justify_content = ftxui::FlexboxConfig::JustifyContent::Center,
      .align_content = ftxui::FlexboxConfig::AlignContent::Center,
  };

  ftxui::Element moves_block = RenderMoves();

  ftxui::Element status_bar =
      RenderStatusBar() | ftxui::hcenter | ftxui::xflex_grow;

  moves_block |= ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 60) |
                 ftxui::size(ftxui::HEIGHT, ftxui::GREATER_THAN, 10);

  static const auto kDecorate = ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 56);

  ftxui::Element chessboard = chessboard_->Render();
  ftxui::Element fen_input = fen_input_->Render() | kDecorate;
  ftxui::Element pgn_input = pgn_input_->Render() | kDecorate;

  ftxui::Element content = ftxui::flexbox(
                               {
                                   status_bar,
                                   ftxui::separatorEmpty(),
                                   ftxui::gridbox({
                                       {
                                           ftxui::vbox({
                                               chessboard | ftxui::center,
                                               ftxui::separatorEmpty(),
                                               fen_input,
                                               pgn_input,
                                           }),
                                           ftxui::separatorEmpty(),
                                           moves_block,
                                       },
                                   }),
                               },
                               config) |
                           ftxui::color(color::gray400) |
                           ftxui::vscroll_indicator | ftxui::frame;

  return content;
}

void Main::InitEngine() {
  std::unique_lock lock(mutex_);

  engine_.Send(uci::command::Input("uci"));

  // TODO: write to log file
  cv_.wait_for(lock, 10s);

  if (!(engine_attrs_ & attrs::UCI)) {
    throw std::runtime_error(
        "engine didn't respond within 10s or isn't uci compatible.");
  }

  if (engine_options_.contains("UCI_AnalyseMode")) {
    SetAndSendOption("UCI_AnalyseMode", true);
  }

  if (engine_options_.contains("MultiPV")) {
    std::int64_t max(5);
    auto &option = engine_options_["MultiPV"];

    option.max = std::min(max, option.max);
    std::int64_t value = option.max;

    SetAndSendOption("MultiPV", value);
  }

  if (engine_options_.contains("Ponder")) {
    SetAndSendOption("Ponder", true);
  }

  if (engine_options_.contains("Threads")) {
    auto &option = engine_options_["Threads"];
    std::int64_t cpus(std::thread::hardware_concurrency());

    option.max = std::min(cpus, option.max);

    std::int64_t value = std::min(static_cast<std::int64_t>(1), option.max);

    SetAndSendOption("Threads", value);
  }

  if (engine_options_.contains("Hash")) {
    std::int64_t max(512);
    auto &option = engine_options_["Hash"];

    option.max = std::min(max, option.max);
    std::int64_t value = std::min(std::get<std::int64_t>(option.value),
                                  std::min(max, option.max));

    SetAndSendOption("Hash", value);
  }

  if (engine_options_.contains("Clear Hash")) {
    engine_attrs_ |= attrs::CLEAR_HASH;
  }

  engine_.Send(uci::command::Input("isready"));

  cv_.wait_for(lock, 10s);

  if (!(engine_attrs_ & attrs::READY)) {
    throw std::runtime_error("engine wasn't ready within 10s.");
  }

  engine_.Send(uci::command::Input("ucinewgame"));
}

void Main::UpdateBoard() {
  for (int i = 0; i < 64; i++) {
    char c = '\0';

    position_.PieceAt(&c, i);
    chessboard_->SetPiece(c, i);
  }
}

void Main::Handle(uci::command::Input *command) {
  switch (command->type) {
    case uci::TokenType::UCI_OK:
      engine_attrs_ |= attrs::UCI;
      break;

    case uci::TokenType::READY_OK:
      engine_attrs_ |= attrs::READY;
      break;

    default:
      break;
  }

  cv_.notify_all();
  ftxui::ScreenInteractive::Active()->PostEvent(ftxui::Event::Custom);
}

void Main::Handle(uci::command::ID *command) {
  switch (command->type) {
    case uci::command::ID::Type::NAME:
      engine_name_ = command->value;

      tui::trim(engine_name_);
      break;

    case uci::command::ID::Type::AUTHOR:
      engine_author_ = command->value;

      tui::trim(engine_author_);
      break;
  }
}

void Main::Handle(uci::command::BestMove *) {}

void Main::Handle(uci::command::CopyProtection *) {}

void Main::Handle(uci::command::Registration *) {}

void Main::Handle(uci::command::Info *command) {
  if (command->multipv < 1) {
    return;
  }

  std::lock_guard lock(mutex_);

  PV &pv = pvs_[command->multipv - 1];

  pv.id = command->multipv;
  pv.nps = command->nps;
  pv.depth = command->depth;
  pv.nps = command->nps;
  pv.value = command->score->value / 100.00;

  pv.moves.clear();

  // TODO: find a way to check if fen is valid before applying
  // engine::Position::ApplyFen(&pv.position, fen_);
  pv.position = position_;

  if (pv.position.Turn() == engine::BLACK) {
    pv.value = -pv.value;
    pv.moves.push_back("...");
  }

  for (const auto &move : command->pv) {
    int from_file = move[0] - 97;
    int from_rank = move[1] - 49;
    int to_file = move[2] - 97;
    int to_rank = move[3] - 49;

    int from = engine::square::From(from_file, from_rank);
    int to = engine::square::From(to_file, to_rank);

    char piece;
    std::string str;

    // INFO: the position might not be valid anymore
    if (!pv.position.PieceAt(&piece, from)) {
      break;
    }

    auto move1 = engine::DeduceMove(pv.position, from, to);

    pv.position.Make(move1);

    if (move1.Is(engine::move::CASTLE_KING_SIDE)) {
      str.append("O-O");
    } else if (move1.Is(engine::move::CASTLE_QUEEN_SIDE)) {
      str.append("O-O-O");
    } else {
      auto [font, _] = kPieceFontsMap[piece];

      str.append(font).append("");

      if (move1.Is(engine::move::CAPTURE)) {
        str.append(kClose);
      }

      str.append(move.substr(2, 2));
    }

    pv.moves.emplace_back(str);
  }

  ftxui::ScreenInteractive::Active()->PostEvent(ftxui::Event::Custom);
}

void Main::Handle(uci::command::Option *command) {
  EngineOption option;
  std::vector<std::string> vars(command->vars.begin(), command->vars.end());

  option.id = command->id;
  option.type = command->type;
  option.min = command->min;
  option.max = command->max;
  option.vars = std::move(vars);

  std::visit(
      [&option](const auto &value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, std::string_view>) {
          option.value.emplace<std::string>(value);
        }

        if constexpr (std::is_integral_v<T>) {
          option.value.emplace<T>(value);
        }
      },
      command->def4ult);

  engine_options_.emplace(command->id, std::move(option));
}

void Main::OnChange(const tui::EngineOption *option) {
  if (running_) {
    running_ = false;
    engine_.Send(uci::command::Input("stop"));
  }

  auto command = option->ToCommand();

  if (command.id == "MultiPV") {
    std::lock_guard lock(mutex_);
    auto it = pvs_.begin() + std::get<std::int64_t>(command.value);

    for (; it != pvs_.end(); it++) {
      if (!it->id) {
        break;
      }

      it->Unset();
    }
  }

  engine_.Send(command);
}

void Main::Start() {
  if (!running_ && (engine_attrs_ & attrs::UCI)) {
    running_ = true;
    engine_.Send(uci::command::Position(fen_));
    engine_.Send(uci::command::Go());
  }
}

void Main::Stop() {
  if (running_) {
    running_ = false;
    engine_.Send(uci::command::Input("stop"));
  }
}

void Main::ClearHash() { engine_.Send(uci::command::SetOption("Clear Hash")); }

void Main::ShowSettings() {
  auto on_change = std::bind(&Main::OnChange, this, std::placeholders::_1);

  auto engine_settings =
      tui::Make<component::EngineSettings>(engine_options_, on_change);

  auto content = ftxui::Renderer(engine_settings, [engine_settings] {
    return engine_settings->Render() | ftxui::borderEmpty;
  });

  engine_settings->Refresh();

  view_->ShowModal(content);
}

void Main::ShowEngineInfo() {
  auto component = ftxui::Renderer([this] {
    return ftxui::vbox({
               ftxui::text("About Engine") | ftxui::hcenter,
               ftxui::separator(),
               ftxui::separatorEmpty(),
               ftxui::text(engine_name_) | ftxui::center,
               ftxui::separatorEmpty(),
               ftxui::text(engine_author_) | ftxui::center,
           }) |
           ftxui::borderEmpty | ftxui::color(color::gray400) |
           ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 50);
  });

  view_->ShowModal(component);
}

ftxui::Element Main::RenderMoves() {
  ftxui::Elements pvs;
  std::lock_guard lock(mutex_);
  auto info = ftxui::emptyElement();

  for (PV &pv : pvs_) {
    if (!pv.id) {
      break;
    }

    pvs.push_back(pv.Render());

    if (&pv != &pvs_.back()) {
      pvs.push_back(ftxui::separator());
    }
  }

  if (pvs_[0].id) {
    auto str = std::format("depth: {} nps: {}", pvs_[0].depth, pvs_[0].nps);
    info = ftxui::text(str);
  }

  return ftxui::vbox({
      ftxui::hbox({ftxui::text("Moves"), ftxui::separator(), info}),
      ftxui::separator(),
      ftxui::vbox(pvs),
  });
}

ftxui::Element Main::RenderStatusBar() {
  component::ModalView::KeyPairs items;

  if (view_->Mode() == tui::NORMAL && engine_attrs_ & attrs::READY) {
    running_ ? items.emplace_back("R", "stop engine")
             : items.emplace_back("r", "run engine");
  }

  if (view_->Mode() == tui::NORMAL && engine_attrs_ & attrs::CLEAR_HASH) {
    items.emplace_back("c", "clear hash");
  }

  if (view_->Mode() == tui::NORMAL && engine_attrs_ & attrs::UCI) {
    items.emplace_back("s", "settings");
    items.emplace_back("a", "engine info");
  }

  return view_->RenderStatusBar(items);
}

bool Main::PV::OnEvent(ftxui::Event &event) {
  if (event.is_mouse()) {
    const auto &mouse = event.mouse();

    if (mouse.motion == ftxui::Mouse::Motion::Pressed &&
        mouse.button == ftxui::Mouse::Button::Left &&
        box.Contain(mouse.x, mouse.y)) {
      show_all_moves = !show_all_moves;

      return true;
    }
  }

  return false;
}

ftxui::Element Main::PV::Render() {
  int index = 1;
  ftxui::Elements els;
  auto it = moves.begin();

  while (it != moves.end()) {
    els.push_back(ftxui::text(std::format("{}. ", index)));
    els.push_back(ftxui::text(*it));

    if (it == moves.end()) {
      break;
    }

    it++;

    if (it != moves.end()) {
      els.push_back(ftxui::text(" " + *it + " "));

      it++;
    }

    index++;
  }

  auto other = ftxui::text(kCaretUp);

  auto el = ftxui::hbox({
                ftxui::text(std::format("{:+.2f}", value)),
                ftxui::separator(),
                ftxui::hflow(els),
            }) |
            ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 55);

  if (!show_all_moves) {
    el |= ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1);

    other = ftxui::text(kCaretDown);
  }

  static const auto config =
      ftxui::FlexboxConfig()
          .Set(ftxui::FlexboxConfig::Wrap::NoWrap)
          .Set(ftxui::FlexboxConfig::Direction::Row)
          .Set(ftxui::FlexboxConfig::JustifyContent::SpaceBetween);

  return ftxui::flexbox(
             {
                 el | ftxui::flex_grow,
                 other | ftxui::flex_shrink,
             },
             config) |
         ftxui::reflect(box);
}

}  // namespace analyze
}  // namespace screen
}  // namespace tui
