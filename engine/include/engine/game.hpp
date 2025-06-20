#ifndef GAME_HPP
#define GAME_HPP

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <variant>

#include <boost/asio.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/process.hpp>
#include <boost/smart_ptr.hpp>

#include "move.hpp"
#include "position.hpp"
#include "types.hpp"

namespace asio = boost::asio;
namespace log = boost::log;
namespace process = boost::process;

typedef void (*UIBindPtr)(const Move &move);
typedef std::function<void(const Move &)> UIBindFn;

class UI {
 public:
  virtual void Refresh() = 0;

  void Bind(UIBindFn);
  void Bind(UIBindPtr);
  void Notify(const Move &move);

 private:
  std::variant<UIBindPtr, UIBindFn> callback_;
};

class Game;

class Player {
 public:
  virtual void Init(const Game &) = 0;
  virtual void Wait(Move &, Game &) = 0;
};

class Human : public Player {
 public:
  void Init(const Game &) override;
  void Wait(Move &, Game &) override;
};

class Engine : public Player {
 public:
  ~Engine();
  Engine(const std::string &binary);

  void Init(const Game &) override;
  void Wait(Move &, Game &) override;

  void Send(const std::string_view &command);
  void Send(std::string &output, const std::string_view &command);
  void Send(const std::string_view &command, const std::string_view clause);
  void Send(std::string &output, const std::string_view &command,
            const std::string_view clause);

 private:
  const std::string &binary_;

  asio::io_context context_;
  process::popen *instance_;

  using Sink = log::sinks::synchronous_sink<log::sinks::text_ostream_backend>;

  log::sources::logger logger_;
  boost::shared_ptr<Sink> sink_;

  void Quit();
  void Ready();
  void NewGame();
  void SetOptions();
  void SwitchToUCI();
  void UpdatePosition(const char *fen);
};

class Game {
 public:
  Position position;
  std::list<Move> moves;
  const std::string_view start_pos;

  Game(Player &white, Player &black, std::string_view position);

  void Init(UI *);
  void Loop();
  void Stop();

  void Restart();
  void UpdateTurns();
  Color GetTurn() const;

 private:
  Player &white_;
  Player &black_;

  Player *opp_;
  Player *turn_;

  bool stop_;
  bool ready_;

  UI *ui_;

  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  friend class Engine;
  friend class Human;
};

#endif
