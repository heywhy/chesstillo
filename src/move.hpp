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

#define MOVE_NORTH_NORTH_EAST(bitboard) (bitboard << 17) & ~kAFile
#define MOVE_NORTH_EAST_EAST(bitboard) (bitboard << 10) & ~(kAFile | kBFile)
#define MOVE_SOUTH_EAST_EAST(bitboard) (bitboard >> 6) & ~(kAFile | kBFile)
#define MOVE_SOUTH_SOUTH_EAST(bitboard) (bitboard >> 15) & ~kAFile
#define MOVE_NORTH_NORTH_WEST(bitboard) (bitboard << 15) & ~kAFile
#define MOVE_NORTH_WEST_WEST(bitboard) (bitboard << 6) & ~kAFile
#define MOVE_SOUTH_WEST_WEST(bitboard) (bitboard >> 10) & ~kAFile
#define MOVE_SOUTH_SOUTH_WEST(bitboard) (bitboard >> 17) & ~kAFile

#define KNIGHT_ATTACKS(bitboard) \
         MOVE_NORTH_NORTH_EAST(bitboard) | MOVE_NORTH_EAST_EAST(bitboard) | \
         MOVE_SOUTH_EAST_EAST(bitboard) | MOVE_SOUTH_SOUTH_EAST(bitboard) | \
         MOVE_NORTH_NORTH_WEST(bitboard) | MOVE_NORTH_WEST_WEST(bitboard) | \
         MOVE_SOUTH_WEST_WEST(bitboard) | MOVE_SOUTH_SOUTH_WEST(bitboard)

#define KING_ATTACKS(bitboard) \
        MOVE_NORTH(bitboard) | MOVE_EAST(bitboard) | MOVE_SOUTH(bitboard) | MOVE_WEST(bitboard)

Bitboard RankMask(int square);
Bitboard FileMask(int square);
Bitboard DiagonalMask(int square);
Bitboard AntiDiagonalMask(int square);

Bitboard BishopAttacks(Bitboard bitboard);
Bitboard RookAttacks(Bitboard bitboard);
Bitboard QueenAttacks(Bitboard bitboard);

void SetFlags(Move *move, ...);

bool IsValidPawnMove(Board &board, Bitboard const piece, Move const &move);

bool IsValidKnightMove(Board &board, Bitboard const piece, Move const &move,
                       Bitboard const &attacking_sqs);

bool IsValidBishopMove(Board &board, Bitboard const piece, Move const &move);
bool IsValidRookMove(Board &board, Bitboard const piece, Move const &move);
bool IsValidQueenMove(Board &board, Bitboard const piece, Move const &move);
#endif
