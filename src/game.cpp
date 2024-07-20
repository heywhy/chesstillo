#include <chesstillo/fen.hpp>
#include <chesstillo/game.hpp>
#include <iostream>
#include <string>

Game::Game() { ApplyFen(board_, START_FEN); };

void Game::Loop() {
  std::string command;

  std::getline(std::cin, command);
};
