#include <iostream>
#include <ostream>

#include "board.hpp"
#include "fen.hpp"

int main(int argc, char **argv) {
  Board board;

  ApplyFen(board, START_FEN);

  while (board.Endgame() == false) {
    std::cout << "enter your move:" << std::endl;

    std::string move;
    std::getline(std::cin, move);

    board.Print();
  }

  return 0;
}
