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
    {"uci", UCI},      {"isready", IS_READY},     {"ucinewgame", UCI_NEW_GAME},
    {"stop", STOP},    {"ponderhit", PONDER_HIT}, {"quit", QUIT},
    {"uciok", UCI_OK}, {"readyok", READY_OK}};
}

} // namespace uci
