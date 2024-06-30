#include <string>

#include "board.hpp"

#ifndef FEN_HPP

#define FEN_HPP

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void ApplyFen(Board & board, const char * fen);
std::string PositionToFen(Board & board);

#endif
