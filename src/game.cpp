#include <chesstillo/game.hpp>
#include <iostream>
#include <string>

#include "fen.hpp"

Game::Game() { ApplyFen(board_, START_FEN); };

void Game::Loop() {
  std::string command;

  std::getline(std::cin, command);
};
