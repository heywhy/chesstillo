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
#include <tui/components.hpp>
#include <tui/config.hpp>
#include <tui/constants.hpp>
#include <tui/hooks.hpp>
#include <tui/screen/analyze.hpp>
#include <tui/theme.hpp>
#include <tui/utility.hpp>

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

using namespace uci;
using namespace std::chrono_literals;

namespace tui {
namespace screen {

Analyze::Analyze(const Theme &theme) : show_engine_settings_(false) {
  auto engine_settings =
      ftxui::Make<component::EngineSettings>(engine_options_);
  auto main =
      ftxui::Make<analyze::Main>(theme, engine_options_, engine_settings.get());

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
}

bool Analyze::OnEvent(ftxui::Event event) {
  if (event == ftxui::Event::CtrlS) {
    show_engine_settings_ = true;

    return true;
  }

  if (event == ftxui::Event::Escape) {
    show_engine_settings_ = false;

    return true;
  }

  return ftxui::ComponentBase::OnEvent(event);
}

namespace analyze {
Main::Main(const Theme &theme, EngineOptions &engine_options,
           component::EngineSettings *engine_settings)
    : theme_(theme),
      engine_options_(engine_options),
      engine_settings_(engine_settings),
      fen_(START_FEN),
      engine_(uci::FindExecutable("stockfish"), this),
      thread_(&Main::InitEngineLoop, this) {
  Add(MakeContainer());
  Add(ftxui::Make<component::Switch, std::array<std::string_view, 2>>(
      {"Stop", "Run"}, running_, std::bind(&Main::OnRunSwitchChange, this)));
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

  std::unique_lock lock(mutex_);

  ftxui::Elements pvs_elements;
  ftxui::Component container = ChildAt(0);

  for (const PV &pv : pvs_) {
    if (pv.id < 1) {
      continue;
    }

    pvs_elements.push_back(pv.Render());

    if (&pv != &pvs_.back()) {
      pvs_elements.push_back(ftxui::separator());
    }
  }

  ftxui::Element moves_block =
      ftxui::vbox({ftxui::hbox({ChildAt(1)->Render()}), ftxui::separator(),
                   ftxui::vbox(pvs_elements)});

  ftxui::Element navigation_bar = ftxui::hbox({
      ftxui::text(" e ") | ftxui::bold,
      ftxui::text("engine settings "),
      ftxui::separator(),
      ftxui::text(" f ") | ftxui::bold,
      ftxui::text("flip board "),
      ftxui::separator(),
      ftxui::text(" c ") | ftxui::bold,
      ftxui::text("continue from here "),
      ftxui::separator(),
      ftxui::text(" q ") | ftxui::bold,
      ftxui::text("quit "),
  });

  ftxui::Element content =
      ftxui::flexbox(
          {ftxui::hbox({navigation_bar | ftxui::borderLight}) | ftxui::hcenter |
               ftxui::xflex_grow,
           ftxui::separatorEmpty(),
           ftxui::gridbox({
               {ftxui::vbox(
                    {container->ChildAt(0)->Render() | ftxui::center,
                     ftxui::separatorEmpty(),
                     container->ChildAt(1)->Render() |
                         ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 62),
                     container->ChildAt(2)->Render() |
                         ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 62)}),
                // ftxui::gaugeUp(0.5) | ftxui::color(ftxui::Color::RosyBrown)
                // |
                //     ftxui::bgcolor(ftxui::Color::GrayLight),
                ftxui::separatorEmpty(),
                moves_block | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 60)},
           })},
          config) |
      ftxui::color(color::gray400) | ftxui::vscroll_indicator | ftxui::frame;

  return content;
}

void Main::InitEngineLoop() {
  std::unique_lock lock(mutex_);

  engine_.Send(command::Input("uci"));

  // TODO: write to log file
  cv_.wait_for(lock, 10s);

  if (!engine_is_uci_compatible_) {
    throw std::runtime_error(
        "engine wasn't ready within 10s or isn't uci compatible.");
  }

  engine_settings_->Refresh();

  if (engine_options_.contains(kUCIAnalyseMode)) {
    engine_.Send(command::SetOption(kUCIAnalyseMode, true));
  }

  if (engine_options_.contains(kMultiPV)) {
    engine_.Send(command::SetOption(kMultiPV, 5));
  }

  engine_.Send(command::Input("isready"));

  cv_.wait_for(lock, 10s);

  if (!engine_is_ready_) {
    throw std::runtime_error("engine wasn't ready within 10s.");
  }

  engine_.Send(command::Input("ucinewgame"));
}

void Main::OnRunSwitchChange() {
  if (running_) {
    engine_.Send(command::Position(fen_));
    engine_.Send(command::Go());
  } else {
    engine_.Send(command::Input("stop"));
  }
}

void Main::Handle(command::Input *command) {
  switch (command->type) {
    case uci::UCI_OK:
      engine_is_uci_compatible_ = true;
      break;

    case uci::READY_OK:
      engine_is_ready_ = true;
      break;

    default:
      break;
  }

  cv_.notify_all();
  ActiveScreen()->PostEvent(ftxui::Event::Custom);
}

void Main::Handle(command::ID *) {}

void Main::Handle(command::BestMove *) {}

void Main::Handle(command::CopyProtection *) {}

void Main::Handle(command::Registration *) {}

void Main::Handle(command::Info *command) {
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

void Main::Handle(command::Option *command) {
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
