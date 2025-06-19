#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <cstddef>
#include <cstdint>

#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>

bool PieceToChar(char *c, Piece piece);
bool PieceToChar(char *c, Piece piece, Color color);
bool ToString(char *buf, const Move &move);
Move DeduceMove(const Position &position, uint8_t from, uint8_t to);
#endif
