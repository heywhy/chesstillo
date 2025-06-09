#include <algorithm>
#include <cstdint>
#include <format>
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
#include <uci/uci.hpp>

#include <tui/color.hpp>
#include <tui/component/modal_view.hpp>
#include <tui/config.hpp>
#include <tui/constants.hpp>
#include <tui/contracts.hpp>
#include <tui/mapping.hpp>
#include <tui/screen/analyze.hpp>
#include <tui/theme.hpp>
#include <tui/types.hpp>
#include <tui/utility.hpp>

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

using namespace std::chrono_literals;

namespace tui {
namespace screen {

Analyze::Analyze(const Theme &theme)
    : component::View(tui::Make<analyze::Main>(theme, this, engine_options_)) {
  // INFO: had to bind manually because of undefined behavior
  tui::HasKeymaps::Bind(dynamic_cast<HasKeymaps *>(main_content_.get()));
}

namespace analyze {
Main::Main(const Theme &theme, component::View *view,
           EngineOptions &engine_options)
    : theme_(theme),
      view_(view),
      engine_options_(engine_options),
      fen_(START_FEN),
      engine_(uci::FindExecutable("stockfish"), this),
      thread_(&Main::InitEngine, this) {
  Add(MakeContainer());
}

Main::~Main() { thread_.join(); }

ftxui::Component Main::MakeContainer() {
  auto fen_input = tui::Make<component::Input>("FEN", fen_);
  auto pgn_input = tui::Make<component::Input>("PGN", pgn_, true);
  auto chessboard = tui::Make<component::Chessboard>(theme_);

  return ftxui::Container::Vertical({chessboard, fen_input, pgn_input});
}

ftxui::Element Main::OnRender() {
  static ftxui::FlexboxConfig config = {
      .direction = ftxui::FlexboxConfig::Direction::Column,
      .justify_content = ftxui::FlexboxConfig::JustifyContent::Center,
      .align_content = ftxui::FlexboxConfig::AlignContent::Center,
  };

  ftxui::Component container = ChildAt(0);
  ftxui::Element moves_block = RenderMoves();

  ftxui::Element status_bar =
      RenderStatusBar() | ftxui::hcenter | ftxui::xflex_grow;

  moves_block |= ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 60) |
                 ftxui::size(ftxui::HEIGHT, ftxui::GREATER_THAN, 10);

  auto decorate = ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 62);

  ftxui::Element chessboard = container->ChildAt(0)->Render();
  ftxui::Element fen_input = container->ChildAt(1)->Render() | decorate;
  ftxui::Element pgn_input = container->ChildAt(2)->Render() | decorate;

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

  if (!(engine_flags_ & engine::UCI)) {
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

    std::int64_t value = std::min(cpus, option.max);

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
    engine_flags_ |= engine::CLEAR_HASH;
  }

  engine_.Send(uci::command::Input("isready"));

  cv_.wait_for(lock, 10s);

  if (!(engine_flags_ & engine::READY)) {
    throw std::runtime_error("engine wasn't ready within 10s.");
  }

  engine_.Send(uci::command::Input("ucinewgame"));
}

void Main::Handle(uci::command::Input *command) {
  switch (command->type) {
    case uci::UCI_OK:
      engine_flags_ |= engine::UCI;
      break;

    case uci::READY_OK:
      engine_flags_ |= engine::READY;
      break;

    default:
      break;
  }

  cv_.notify_all();
  ftxui::ScreenInteractive::Active()->PostEvent(ftxui::Event::Custom);
}

void Main::Handle(uci::command::ID *command) {
  switch (command->type) {
    case uci::command::ID::NAME:
      engine_name_ = command->value;

      tui::trim(engine_name_);
      break;

    case uci::command::ID::AUTHOR:
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

  std::unique_lock lock(mutex_);

  PV &pv = pvs_[command->multipv - 1];

  pv.id = command->multipv;
  pv.nps = command->nps;
  pv.depth = command->depth;
  pv.score = *command->score;
  pv.value = command->score->value / 100.00;

  pv.moves.clear();

  for (const auto &move : command->pv) {
    pv.moves.emplace_back(move);
  }

  ftxui::ScreenInteractive::Active()->PostEvent(ftxui::Event::Custom);
}

void Main::Handle(uci::command::Option *command) {
  auto on_change = [this](uci::command::SetOption &command) {
    if (running_) {
      running_ = false;
      engine_.Send(uci::command::Input("stop"));
    }

    if (command.id == "MultiPV") {
      std::unique_lock lock(mutex_);
      auto it = pvs_.begin() + std::get<std::int64_t>(command.value);

      for (; it != pvs_.end(); it++) {
        if (it->id < 1) {
          break;
        }

        it->Unset();
      }
    }

    engine_.Send(command);
  };

  EngineOption option(on_change);
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

  engine_options_.emplace(std::pair(option.id, std::move(option)));
}

void Main::BindKeymaps() {
  view_->SetKeymap(tui::NORMAL, "a", [this] {
    auto component = ftxui::Renderer([this] {
      return ftxui::vbox({
                 ftxui::text("About Engine") | ftxui::hcenter,
                 ftxui::separator(),
                 ftxui::separatorEmpty(),
                 ftxui::text(engine_name_) | ftxui::center,
                 ftxui::separatorEmpty(),
                 ftxui::text(engine_author_) | ftxui::center,
             }) |
             ftxui::borderEmpty | ftxui::color(color::gray400);
    });

    view_->ShowModal(component);
  });

  view_->SetKeymap(tui::NORMAL, "s", [this] {
    auto engine_settings =
        tui::Make<component::EngineSettings>(engine_options_);

    auto content = ftxui::Renderer(engine_settings, [engine_settings] {
      return engine_settings->Render() | ftxui::borderEmpty;
    });

    engine_settings->Refresh();

    view_->ShowModal(content);
  });

  view_->SetKeymap(tui::NORMAL, "c", [this] {
    engine_.Send(uci::command::SetOption("Clear Hash"));
  });

  view_->SetKeymap(tui::NORMAL, "r", [this] {
    if (!running_) {
      running_ = true;
      engine_.Send(uci::command::Position(fen_));
      engine_.Send(uci::command::Go());
    }
  });

  view_->SetKeymap(tui::NORMAL, "R", [this] {
    if (running_) {
      running_ = false;
      engine_.Send(uci::command::Input("stop"));
    }
  });
}

ftxui::Element Main::RenderMoves() {
  ftxui::Elements pvs;
  std::unique_lock lock(mutex_);

  for (const PV &pv : pvs_) {
    if (pv.id < 1) {
      continue;
    }

    pvs.push_back(pv.Render());

    if (&pv != &pvs_.back()) {
      pvs.push_back(ftxui::separator());
    }
  }

  return ftxui::vbox({
      ftxui::hbox({ftxui::text("Moves")}),
      ftxui::separator(),
      ftxui::vbox(pvs),
  });
}

ftxui::Element Main::RenderStatusBar() {
  component::ModalView::KeyPairs items;

  if (view_->Mode() == tui::NORMAL && engine_flags_ & engine::READY) {
    running_ ? items.emplace_back("R", "stop engine")
             : items.emplace_back("r", "run engine");
  }

  if (view_->Mode() == tui::NORMAL && engine_flags_ & engine::CLEAR_HASH) {
    items.emplace_back("c", "clear hash");
  }

  if (view_->Mode() == tui::NORMAL && engine_flags_ & engine::UCI) {
    items.emplace_back("s", "settings");
    items.emplace_back("a", "engine info");
  }

  return view_->RenderStatusBar(items);
}

ftxui::Element Main::PV::Render() const {
  int index = 1;
  ftxui::Elements attrs;
  auto it = moves.begin();

  while (it != moves.end()) {
    ftxui::Elements els;

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

    attrs.push_back(ftxui::hbox(els));
  }

  return ftxui::hbox({ftxui::text(std::format("{:.2f} ", value)),
                      ftxui::separator(), ftxui::hflow(attrs)});
}

}  // namespace analyze
}  // namespace screen
}  // namespace tui
