#include <chesstillo/board.hpp>
#include <chesstillo/fen.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "components/chessboard.hpp"

int main() {
  Board board;

  ApplyFen(board, START_FEN);

  ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

  screen.TrackMouse();
  screen.Loop(ftxui::Make<ChessBoard>(std::move(board)));

  return 0;
}
