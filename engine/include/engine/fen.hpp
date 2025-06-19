#ifndef FEN_HPP
#define FEN_HPP

#include <string>

#include "position.hpp"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void ApplyFen(Position &position, const char *fen);
std::string PositionToFen(Position &position);

#endif
