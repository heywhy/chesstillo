#include <boost/asio.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/process.hpp>
#include <boost/smart_ptr.hpp>
#include <condition_variable>
#include <engine/fen.hpp>
#include <engine/game.hpp>
#include <engine/move.hpp>
#include <engine/types.hpp>
#include <format>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <variant>

namespace asio = boost::asio;
namespace log = boost::log;
namespace process = boost::process;
namespace bsystem = boost::system;

using namespace std::chrono_literals;

void UI::Notify(const Move &move) {
  // INFO: this check is important just to make sure that a callback exists
  try {
    if (std::get<0>(callback_) == nullptr) {
      return;
    }
  } catch (const std::bad_variant_access &) {
  }

  std::visit([&](auto fn) { fn(move); }, callback_);

  callback_ = nullptr;
}

void UI::Bind(UIBindFn fn) { callback_ = fn; }
void UI::Bind(UIBindPtr fn) { callback_ = fn; }

Game::Game(Player &white, Player &black, std::string_view start_pos)
    : start_pos(start_pos),
      white_(white),
      black_(black),
      stop_(true),
      ready_(false),
      thread_(&Game::Loop, this) {}

// TODO: don't accept a pointer?
void Game::Init(UI *ui) {
  Game &instance = *this;
  std::unique_lock lock(mutex_);

  ui_ = ui;
  white_.Init(instance);
  black_.Init(instance);

  ApplyFen(position, start_pos.data());

  UpdateTurns();

  ui_->Refresh();

  ready_ = true;
  stop_ = false;

  lock.unlock();
  cv_.notify_all();
}

Color Game::GetTurn() const { return position.GetTurn(); }

void Game::UpdateTurns() {
  Color color = position.GetTurn();

  if (color == WHITE && turn_ != &white_) {
    turn_ = &white_;
    opp_ = &black_;
  } else if (color == BLACK && turn_ != &black_) {
    opp_ = &white_;
    turn_ = &black_;
  }
}

void Game::Restart() {
  std::unique_lock lock(mutex_);

  ApplyFen(position, start_pos.data());

  moves.clear();
  UpdateTurns();

  ui_->Refresh();
}

void Game::Loop() {
  std::unique_lock lock(mutex_);

  Move move(Move::NONE);

  cv_.wait(lock, [&] { return ready_; });

  lock.unlock();

  while (!stop_) {
    turn_->Wait(move, *this);

    lock.lock();

    if (move != Move::NONE) {
      position.Make(move);
      moves.push_front(move);

      move = Move::NONE;

      std::swap(turn_, opp_);

      ui_->Refresh();
    }

    lock.unlock();
  }
}

void Game::Stop() {
  std::unique_lock lock(mutex_);

  stop_ = true;
  ready_ = false;

  lock.unlock();
  cv_.notify_all();

  thread_.join();
}

void Human::Init(const Game &) {}

void Human::Wait(Move &move, Game &game) {
  bool free = false;
  std::unique_lock lock(game.mutex_);

  auto fn = [&](const Move &temp) {
    free = true;
    move = temp;

    game.cv_.notify_all();
  };

  game.ui_->Bind(fn);

  game.cv_.wait(lock, [&] { return free || game.stop_ || game.turn_ != this; });
};

Engine::Engine(const std::string &binary)
    : binary_(binary), instance_(nullptr), sink_(boost::make_shared<Sink>()) {
  // TODO: update logfile
  std::string log_file("/Users/heywhy/Dev/me/chesstillo/build/engine.log");
  auto log_file_stream = boost::make_shared<std::ofstream>(log_file);

  sink_->locked_backend()->add_stream(log_file_stream);

  log::core::get()->add_sink(sink_);
}

Engine::~Engine() {
  if (instance_ != nullptr && instance_->running()) {
    Quit();

    instance_->terminate();

    delete instance_;
  }

  log::core::get()->remove_sink(sink_);
}

// TODO:
// * throw exception if the process couldn't be started.
// * get supported options and forward to the running process
void Engine::Init(const Game &) {
  std::string output;
  instance_ = new process::popen(context_.get_executor(), binary_, {});

  SwitchToUCI();
  SetOptions();
  NewGame();
  Ready();
}

void Engine::Wait(Move &, Game &game) {
  bool free = false;
  std::string output;
  std::unique_lock lock(game.mutex_);
  std::string fen = PositionToFen(game.position);

  UpdatePosition(fen.c_str());
  Send(output, "go depth 8", "bestmove");

  BOOST_LOG(logger_) << "============";
  BOOST_LOG(logger_) << output;
  BOOST_LOG(logger_) << "============";

  // game.cv_.notify_all();

  game.cv_.wait(lock, [&] { return free || game.stop_ || game.turn_ != this; });
}

void Engine::SwitchToUCI() {
  // TODO: parse options from output
  Send("uci", "uciok");
}

void Engine::SetOptions() {}

void Engine::UpdatePosition(const char *fen) {
  Send(std::format("position fen {}", fen));
}

void Engine::NewGame() { Send("ucinewgame"); }

void Engine::Ready() { Send("isready", "readyok"); }

void Engine::Quit() { Send("quit"); }

void Engine::Send(const std::string_view &command) {
  std::string str(command);
  str = str.append("\n");

  asio::write(*instance_, asio::buffer(str));
}

void Engine::Send(std::string &output, const std::string_view &command) {
  bsystem::error_code error;

  output.clear();

  Send(command);

  asio::read(*instance_, asio::dynamic_buffer(output),
             asio::transfer_at_least(1), error);

  BOOST_LOG(logger_) << output;
}

void Engine::Send(const std::string_view &command,
                  const std::string_view clause) {
  std::string output;

  Send(output, command, clause);
}

void Engine::Send(std::string &output, const std::string_view &command,
                  const std::string_view clause) {
  bsystem::error_code error;

  output.clear();

  Send(command);

  asio::read_until(*instance_, asio::dynamic_buffer(output), clause, error);

  BOOST_LOG(logger_) << output;
}
