#include <format>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <ftxui/component/component.hpp>
#include <tui/components.hpp>
#include <tui/config.hpp>
#include <tui/constants.hpp>
#include <tui/hooks.hpp>
#include <tui/screen/analyze.hpp>
#include <tui/theme.hpp>
#include <tui/utility.hpp>
#include <uci/uci.hpp>

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

using namespace uci;
using namespace std::chrono_literals;

namespace tui {
namespace screen {

Analyze::Analyze(const Theme &theme)
    : theme_(theme), fen_(START_FEN),
      engine_(uci::FindExecutable("stockfish"), this),
      thread_(&Analyze::InitEngineLoop, this) {
  Add(MakeContainer());
  Add(ftxui::Make<component::Switch, std::array<std::string_view, 2>>(
      {"Stop", "Run"}, running_, std::bind(&Analyze::OnRunSwitchChange, this)));
  Add(ftxui::Make<component::EngineSettings>(engine_options_));
}

Analyze::~Analyze() { thread_.join(); }

ftxui::Component Analyze::MakeContainer() {
  auto fen_input = ftxui::Make<component::Input>("FEN", fen_);
  auto pgn_input = ftxui::Make<component::Input>("PGN", pgn_, true);
  auto chessboard = ftxui::Make<component::Chessboard>(theme_);

  return ftxui::Container::Vertical({chessboard, fen_input, pgn_input});
}

ftxui::Element Analyze::OnRender() {
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

  ftxui::Element moves_block = ftxui::vbox(
      {ftxui::hbox({ChildAt(1)->Render(), ftxui::separator(),
                    ftxui::text(std::format("{:L}n/s", pvs_[0].nps))}),
       // ftxui::text(engine_is_ready_ ? "engine ready" : "moves block"),
       ftxui::separator(), ftxui::vbox(pvs_elements)});

  ftxui::Element content =
      ftxui::flexbox(
          {ftxui::text("on:top") | ftxui::bgcolor(ftxui::Color::Red) |
               ftxui::xflex_grow,
           ftxui::emptyElement(),
           ftxui::gridbox(
               {{container->ChildAt(0)->Render(),
                 // ftxui::gaugeUp(0.5) | ftxui::color(ftxui::Color::RosyBrown)
                 // |
                 //     ftxui::bgcolor(ftxui::Color::GrayLight),
                 moves_block | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 60)},
                {ftxui::separatorEmpty()},
                {container->ChildAt(1)->Render() |
                 ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 62)},
                {container->ChildAt(2)->Render() |
                 ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 62)}})},
          config) |
      ftxui::color(ftxui::Color::GrayLight) | ftxui::vscroll_indicator |
      ftxui::frame;

  if (show_engine_settings_) {
    content = ftxui::dbox(
        {content, ChildAt(2)->Render() | ftxui::clear_under | ftxui::center});
  }

  return content;
}

bool Analyze::OnEvent(ftxui::Event event) {
  return ftxui::ComponentBase::OnEvent(event);
}

void Analyze::InitEngineLoop() {
  std::unique_lock lock(mutex_);

  engine_.Send(command::Input("uci"));

  // TODO: write to log file
  cv_.wait_for(lock, 5s);

  if (!engine_is_uci_compatible_) {
    throw std::runtime_error(
        "engine wasn't ready within 5s or isn't uci compatible.");
  }

  static_cast<component::EngineSettings *>(ChildAt(2).get())->Refresh();

  if (engine_options_.contains(kUCIAnalyseMode)) {
    engine_.Send(command::SetOption(kUCIAnalyseMode, "true"));
  }

  if (engine_options_.contains(kMultiPV)) {
    engine_.Send(command::SetOption(kMultiPV, "5"));
  }

  engine_.Send(command::Input("isready"));

  cv_.wait_for(lock, 5s);

  if (!engine_is_ready_) {
    throw std::runtime_error("engine wasn't ready within 5s.");
  }

  engine_.Send(command::Input("ucinewgame"));
}

void Analyze::OnRunSwitchChange() {
  if (running_) {
    engine_.Send(command::Position(fen_));
    engine_.Send(command::Go());
  } else {
    engine_.Send(command::Input("stop"));
  }
}

void Analyze::Handle(command::Input *command) {
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

void Analyze::Handle(command::ID *) {}

void Analyze::Handle(command::BestMove *) {}

void Analyze::Handle(command::CopyProtection *) {}

void Analyze::Handle(command::Registration *) {}

void Analyze::Handle(command::Info *command) {
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

void Analyze::Handle(command::Option *command) {
  EngineOption option;
  std::vector<std::string> vars(command->vars.begin(), command->vars.end());

  option.type = command->type;
  option.value = command->def4ult;
  option.min = command->min;
  option.max = command->max;
  option.vars = std::move(vars);

  engine_options_.emplace(std::pair(command->id, std::move(option)));
}

ftxui::Element Analyze::PV::Render() const {
  ftxui::Elements attrs;

  // attrs.push_back(ftxui::text(std::format("{:.2f} ", pv.value)));
  // attrs.push_back(ftxui::separator());

  int index = 1;
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

  // for (const auto &move : pv.moves) {
  //   attrs.push_back(ftxui::text(" "));
  //   attrs.push_back(ftxui::text(move));
  // }

  // pvs_elements.push_back(ftxui::hflow(attrs));

  return ftxui::hbox({ftxui::text(std::format("{:.2f} ", value)),
                      ftxui::separator(), ftxui::hflow(attrs)});
}

} // namespace screen
} // namespace tui
