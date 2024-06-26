#include <string>

#include "board.hpp"

#ifndef FEN_HPP

#define FEN_HPP

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

Board &ApplyFen(Board &, const char *);
std::string PositionToFen(Board &);

#endif
