#include "board.hpp"
#include "constants.hpp"
#include "types.hpp"

#ifndef MOVE_HPP

#define MOVE_HPP

#define MOVE_NORTH(bitboard) bitboard << 8
#define MOVE_SOUTH(bitboard) bitboard >> 8
#define MOVE_EAST(bitboard) (bitboard << 1) & ~kAFile
#define MOVE_NORTH_EAST(bitboard) (bitboard << 9) & ~kAFile
#define MOVE_SOUTH_EAST(bitboard) (bitboard >> 7) & ~kAFile
#define MOVE_WEST(bitboard) (bitboard >> 1) & ~kHFile
#define MOVE_SOUTH_WEST(bitboard) (bitboard >> 9) & ~kHFile
#define MOVE_NORTH_WEST(bitboard) (bitboard << 7) & ~kHFile

bool IsValidPawnMove(Board &board, Bitboard piece, Move const &move);
#endif
