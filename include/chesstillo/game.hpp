#ifndef GAME_HPP
#define GAME_HPP

#include "position.hpp"

class Game {
public:
  Game();
  void Loop();

private:
  Position position_;
};

#endif
