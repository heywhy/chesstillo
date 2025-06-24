#include <string_view>
#include <unordered_map>

#include <boost/process/v2/environment.hpp>

#include <uci/command.hpp>
#include <uci/types.hpp>

namespace process = boost::process;

namespace uci {
std::unordered_map<std::string_view, Status> kStatus = {
    {"ok", Status::OK},
    {"error", Status::ERROR},
    {"checking", Status::CHECKING}};

std::string FindExecutable(const std::string_view &executable) {
  return process::environment::find_executable(executable).string();
}

namespace command {
std::unordered_map<std::string_view, TokenType> Input::Known = {
    {"uci", TokenType::UCI},
    {"isready", TokenType::IS_READY},
    {"ucinewgame", TokenType::UCI_NEW_GAME},
    {"stop", TokenType::STOP},
    {"ponderhit", TokenType::PONDER_HIT},
    {"quit", TokenType::QUIT},
    {"uciok", TokenType::UCI_OK},
    {"readyok", TokenType::READY_OK}};
}

}  // namespace uci
