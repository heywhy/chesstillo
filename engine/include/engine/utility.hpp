#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <cstddef>
#include <cstdint>

#include "position.hpp"
#include "types.hpp"

namespace engine {

bool PieceToChar(char *c, Piece piece);
bool PieceToChar(char *c, Piece piece, Color color);
bool ToString(char *buf, const Move &move);
Move DeduceMove(const Position &position, std::uint_fast8_t from,
                std::uint_fast8_t to);

}  // namespace engine

#endif
