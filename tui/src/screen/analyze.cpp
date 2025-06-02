#include <format>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <ftxui/component/component.hpp>
#include <uci/uci.hpp>

#include <tui/color.hpp>
#include <tui/component/modal_view.hpp>
#include <tui/config.hpp>
#include <tui/constants.hpp>
#include <tui/hooks.hpp>
#include <tui/screen/analyze.hpp>
#include <tui/theme.hpp>
#include <tui/types.hpp>
#include <tui/utility.hpp>

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

using namespace std::chrono_literals;

namespace tui {
namespace screen {

Analyze::Analyze(const Theme &theme) : show_engine_settings_(false) {
  auto engine_settings =
      ftxui::Make<component::EngineSettings>(engine_options_);
  auto main = ftxui::Make<analyze::Main>(theme, this, engine_options_,
                                         engine_settings.get());

  auto scroll_view = ftxui::Make<component::ScrollView>(engine_settings);

  auto modal = ftxui::Renderer(scroll_view, [scroll_view]() {
    auto content = scroll_view->Render() |
                   ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 80) |
                   ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 40);

    return ftxui::window(ftxui::text("Engine Settings") | ftxui::bold, content,
                         ftxui::BorderStyle::LIGHT) |
           ftxui::color(ftxui::Color::GrayLight);
  });

  Add(ftxui::Modal(main, modal, &show_engine_settings_));

  // INFO: dummy bindings
  SetKeymap(NORMAL, "e", [this] { show_engine_settings_ = true; });
  SetKeymap(NORMAL, "<esc>", [this] { show_engine_settings_ = false; });
}

namespace analyze {
Main::Main(const Theme &theme, component::ModalView *modal_view,
           EngineOptions &engine_options,
           component::EngineSettings *engine_settings)
    : theme_(theme),
      modal_view_(modal_view),
      engine_options_(engine_options),
      engine_settings_(engine_settings),
      fen_(START_FEN),
      engine_(uci::FindExecutable("stockfish"), this),
      thread_(&Main::InitEngineLoop, this) {
  Add(MakeContainer());
}

Main::~Main() { thread_.join(); }

ftxui::Component Main::MakeContainer() {
  auto fen_input = ftxui::Make<component::Input>("FEN", fen_);
  auto pgn_input = ftxui::Make<component::Input>("PGN", pgn_, true);
  auto chessboard = ftxui::Make<component::Chessboard>(theme_);

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

void Main::InitEngineLoop() {
  std::unique_lock lock(mutex_);

  engine_.Send(uci::command::Input("uci"));

  // TODO: write to log file
  cv_.wait_for(lock, 10s);

  if (!(engine_flags_ & engine::UCI)) {
    throw std::runtime_error(
        "engine didn't respond within 10s or isn't uci compatible.");
  }

  engine_settings_->Refresh();

  if (engine_options_.contains("UCI_AnalyseMode")) {
    engine_.Send(uci::command::SetOption("UCI_AnalyseMode", true));
  }

  if (engine_options_.contains("MultiPV")) {
    engine_.Send(uci::command::SetOption("MultiPV", 5));
  }

  if (engine_options_.contains("Ponder")) {
    engine_flags_ |= engine::PONDER;

    engine_.Send(uci::command::SetOption("Ponder", true));
  }

  engine_.Send(uci::command::Input("isready"));

  cv_.wait_for(lock, 10s);

  if (!(engine_flags_ & engine::READY)) {
    throw std::runtime_error("engine wasn't ready within 10s.");
  }

  engine_.Send(uci::command::Input("ucinewgame"));

  BindKeymaps();
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
  ActiveScreen()->PostEvent(ftxui::Event::Custom);
}

void Main::Handle(uci::command::ID *) {}

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

  ActiveScreen()->PostEvent(ftxui::Event::Custom);
}

void Main::Handle(uci::command::Option *command) {
  EngineOption option;
  std::vector<std::string> vars(command->vars.begin(), command->vars.end());

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

  engine_options_.emplace(std::pair(command->id, std::move(option)));
}

void Main::BindKeymaps() {
  modal_view_->SetKeymap(tui::NORMAL, "r", [this] {
    if (running_) {
      return;
    }

    running_ = true;
    engine_.Send(uci::command::Position(fen_));
    engine_.Send(uci::command::Go());
  });

  modal_view_->SetKeymap(tui::NORMAL, "s", [this] {
    if (!running_) {
      return;
    }

    running_ = false;
    engine_.Send(uci::command::Input("stop"));
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

  return ftxui::vbox({ftxui::hbox({ftxui::text("Moves")}), ftxui::separator(),
                      ftxui::vbox(pvs)});
}

ftxui::Element Main::RenderStatusBar() {
  component::ModalView::KeyPairs items;

  if (modal_view_->Mode() == tui::NORMAL && engine_flags_ & engine::READY) {
    running_ ? items.emplace_back("s", "stop engine")
             : items.emplace_back("r", "run engine");
  }

  return modal_view_->RenderStatusBar(items);
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
