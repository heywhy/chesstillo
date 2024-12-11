#include <chesstillo/board.hpp>
#include <chesstillo/utility.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "utils.hpp"

void HookQuitEvent(ftxui::Component &component) {
  component |= ftxui::CatchEvent([&](ftxui::Event event) {
    auto screen = ftxui::ScreenInteractive::Active();

    if (event == ftxui::Event::q && screen != nullptr) {
      screen->ExitLoopClosure()();

      return true;
    }

    return false;
  });
}

bool ToString(char *text, const Move &move, Color turn) {
  Coord to;
  Coord from;

  if (!CoordForSquare(&from, move.from) || !CoordForSquare(&to, move.to)) {
    return false;
  }

  int i = 0;
  char piece;
  char buffer[6];

  if (move.piece != PAWN && PieceToChar(&piece, move.piece, turn)) {
    buffer[i++] = piece;

    if (move.Is(CAPTURE)) {
      buffer[i++] = 'x';
    }

    buffer[i++] = to.file;
  } else if (move.Is(CAPTURE)) {
    buffer[i++] = from.file;
    buffer[i++] = 'x';
    buffer[i++] = to.file;
  } else {
    buffer[i++] = to.file;
  }

  buffer[i++] = 48 + to.rank;

  if (move.Is(CHECK)) {
    buffer[i++] = '+';
  }

  if (move.Is(CHECKMATE)) {
    buffer[i - 1] = '#';
  }

  buffer[i++] = '\0';

  std::strcpy(text, buffer);

  return true;
}
