#ifndef UCI_HPP
#define UCI_HPP

#include "command.hpp"
#include "engine.hpp"
#include "parser.hpp"
#include "scanner.hpp"

namespace uci {
std::string FindExecutable(const std::string_view &exe);
}

#endif
