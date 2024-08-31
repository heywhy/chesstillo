#ifndef TUI_CONTRACTS_HPP
#define TUI_CONTRACTS_HPP

#include <cstdint>

#include <chesstillo/types.hpp>

class ChessboardListener {
public:
  virtual void OnMove(Move &move) = 0;
};

class SquareListener {
public:
  virtual void OnSelect(std::uint8_t index) = 0;
};
#endif
