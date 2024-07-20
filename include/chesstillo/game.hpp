#include "board.hpp"

#ifndef GAME_HPP

#define GAME_HPP

class Game {
public:
  Game();
  void Loop();

private:
  Board board_;
};

#endif
