#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <cstddef>

#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>

std::size_t Split(Bitboard bb, Bitboard *const out);

bool PieceToChar(Piece piece, char *c);
bool PieceToChar(Piece piece, Color color, char *c);
bool MoveToString(Move const &move, Color turn, char *text);
Move DeduceMove(Position &position, unsigned int from, unsigned int to);
#endif
