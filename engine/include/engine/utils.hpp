#ifndef ENGINE_UTILS_HPP
#define ENGINE_UTILS_HPP

#include <cstddef>

#include "position.hpp"
#include "types.hpp"

namespace engine {

bool PieceToChar(char *c, Piece piece);
bool PieceToChar(char *c, Piece piece, Color color);
bool ToString(char *buf, const Move &move);
Move DeduceMove(const Position &position, int from, int to);

}  // namespace engine

#endif
