#include <cstdint>
#include <string>

#include <chesstillo/board.hpp>
#include <chesstillo/fen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>

int ToInt(char num) { return num - '0'; }

void ApplyFen(Position &position, const char *fen) {
  Bitboard *piece;
  Bitboard *black_pieces = position.Pieces(BLACK);
  Bitboard *white_pieces = position.Pieces(WHITE);

  int rank = 7;
  int file = 0;
  int spaces = 0;
  long move_count = 0;
  int en_passant_rank;
  char en_passant_file;

  position.Reset();

  while (*fen) {
    piece = nullptr;

    switch (*fen) {
    case 'r':
      piece = &black_pieces[ROOK];
      break;

    case 'n':
      piece = &black_pieces[KNIGHT];
      break;

    case 'b':
      piece = &black_pieces[BISHOP];
      position.turn_ = BLACK;
      en_passant_file = *fen;
      break;

    case 'q':
      piece = &black_pieces[QUEEN];
      if (spaces == 2)
        position.castling_rights_ |= static_cast<std::uint8_t>(1) << Q_BLACK;
      break;

    case 'k':
      piece = &black_pieces[KING];
      if (spaces == 2)
        position.castling_rights_ |= static_cast<std::uint8_t>(1) << K_BLACK;
      break;

    case 'p':
      piece = &black_pieces[PAWN];
      break;

    case 'R':
      piece = &white_pieces[ROOK];
      break;

    case 'N':
      piece = &white_pieces[KNIGHT];
      break;

    case 'B':
      piece = &white_pieces[BISHOP];
      break;

    case 'Q':
      piece = &white_pieces[QUEEN];
      if (spaces == 2)
        position.castling_rights_ |= static_cast<std::uint8_t>(1) << Q_WHITE;
      break;

    case 'K':
      piece = &white_pieces[KING];

      if (spaces == 2)
        position.castling_rights_ |= static_cast<std::uint8_t>(1) << K_WHITE;
      break;

    case 'P':
      piece = &white_pieces[PAWN];
      break;

    case 'a':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
      en_passant_file = *fen;
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
      file += ToInt(*fen);
      en_passant_rank = ToInt(*fen);
      move_count = ToInt(*fen) + (move_count * 10);
      break;

    case '0':
    case '9':
      move_count = ToInt(*fen) + (move_count * 10);
      break;

    case '/':
      rank--;
      file = 0;
      break;

    case 'w':
      position.turn_ = WHITE;
      break;

    case ' ':
      spaces++;
      move_count = 0;
      en_passant_rank = 0;
      en_passant_file = 0;
      break;
    }

    if (spaces == 0 && piece) {
      unsigned int square = TO_SQUARE(file, rank);
      *piece |= BITBOARD_FOR_SQUARE(square);

      file++;
    } else if (spaces == 3 && (en_passant_rank == 3 || en_passant_rank == 6) &&
               en_passant_file >= 'a' && en_passant_file <= 'h') {
      unsigned int rank = en_passant_rank - 1;
      unsigned int file = en_passant_file - 97;
      unsigned int square = TO_SQUARE(file, rank);

      position.en_passant_sq_ = BITBOARD_FOR_SQUARE(square);
    } else if (spaces == 4) {
      position.halfmove_clock_ = move_count;
    } else if (spaces == 5) {
      position.fullmove_counter_ = move_count;
    }

    fen++;
  }

  position.UpdateInternals();
}

std::string PositionToFen(Position &position) {
  int spaces = 0;
  std::string fen;
  Bitboard occupied_sqs = position.OccupiedSquares();

  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      unsigned int square = TO_SQUARE(file, rank);
      Bitboard bb = BITBOARD_FOR_SQUARE(square);

      if (occupied_sqs & bb) {
        if (spaces > 0) {
          fen += ('0' + spaces);
          spaces = 0;
        }

        char piece;

        if (position.PieceAtSquare(square, &piece)) {
          fen += piece;
        }
      } else {
        spaces++;
      }
    }

    if (spaces > 0) {
      fen += ('0' + spaces);
      spaces = 0;
    }

    if (rank != 0)
      fen += '/';
  }

  if (position.turn_ == WHITE) {
    fen += {' ', 'w', ' '};
  } else {
    fen += {' ', 'b', ' '};
  }

  if (position.CanCastle(K_WHITE)) {
    fen += 'K';
  }
  if (position.CanCastle(Q_WHITE)) {
    fen += 'Q';
  }
  if (position.CanCastle(K_BLACK)) {
    fen += 'k';
  }
  if (position.CanCastle(Q_BLACK)) {
    fen += 'q';
  }

  if (fen.ends_with(' ')) {
    fen += '-';
  }

  Coord coord;
  unsigned int en_passant_square = BIT_INDEX(position.en_passant_sq_);

  if (CoordForSquare(&coord, en_passant_square)) {
    char rank = '0' + coord.rank;

    fen += {' ', coord.file, rank, ' '};
  } else {
    fen += {' ', '-', ' '};
  }

  return fen += std::to_string(position.halfmove_clock_) + ' ' +
                std::to_string(position.fullmove_counter_);
}
