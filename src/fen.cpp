#include <cstdint>
#include <string>

#include "board.hpp"
#include "fen.hpp"

int ToInt(char num) { return num - '0'; }

void ApplyFen(Board &board, const char *fen) {
  Bitboard *piece;

  int rank = 7;
  int file = 0;
  int spaces = 0;
  long move_count = 0;
  int en_passant_rank;
  char en_passant_file;

  board.Reset();

  while (*fen) {
    piece = NULL;

    switch (*fen) {
    case 'r':
      piece = &board.b_pieces_[ROOK];
      break;

    case 'n':
      piece = &board.b_pieces_[KNIGHT];
      break;

    case 'b':
      piece = &board.b_pieces_[BISHOP];
      board.turn_ = BLACK;
      en_passant_file = *fen;
      break;

    case 'q':
      piece = &board.b_pieces_[QUEEN];
      if (spaces == 2)
        board.castling_rights_ |= static_cast<std::uint8_t>(1) << Q_BLACK;
      break;

    case 'k':
      piece = &board.b_pieces_[KING];
      if (spaces == 2)
        board.castling_rights_ |= static_cast<std::uint8_t>(1) << K_BLACK;
      break;

    case 'p':
      piece = &board.b_pieces_[PAWN];
      break;

    case 'R':
      piece = &board.w_pieces_[ROOK];
      break;

    case 'N':
      piece = &board.w_pieces_[KNIGHT];
      break;

    case 'B':
      piece = &board.w_pieces_[BISHOP];
      break;

    case 'Q':
      piece = &board.w_pieces_[QUEEN];
      if (spaces == 2)
        board.castling_rights_ |= static_cast<std::uint8_t>(1) << Q_WHITE;
      break;

    case 'K':
      piece = &board.w_pieces_[KING];

      if (spaces == 2)
        board.castling_rights_ |= static_cast<std::uint8_t>(1) << K_WHITE;
      break;

    case 'P':
      piece = &board.w_pieces_[PAWN];
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
      board.turn_ = WHITE;
      break;

    case ' ':
      spaces++;
      move_count = 0;
      en_passant_rank = 0;
      en_passant_file = 0;
      break;
    }

    if (spaces == 0 && piece) {
      *piece |= BitboardForSquare(file, rank);

      file++;
    } else if (spaces == 3 && (en_passant_rank == 3 || en_passant_rank == 6) &&
               en_passant_file >= 'a' && en_passant_file <= 'h') {
      board.en_passant_sq_ =
          BitboardForSquare(en_passant_file, en_passant_rank);
    } else if (spaces == 4) {
      board.halfmove_clock_ = move_count;
    } else if (spaces == 5) {
      board.fullmove_counter_ = move_count;
    }

    fen++;
  }

  board.ComputeOccupiedSqs();
  board.ComputeAttackedSqs();
}

std::string PositionToFen(Board &board) {
  int spaces = 0;
  std::string fen;

  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      Bitboard square = BitboardForSquare(file, rank);

      if (board.occupied_sqs_ & square) {
        if (spaces > 0) {
          fen += ('0' + spaces);
          spaces = 0;
        }

        fen += board.PieceAtSquare(square);
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

  if (board.turn_ == WHITE) {
    fen += {' ', 'w', ' '};
  } else {
    fen += {' ', 'b', ' '};
  }

  if (board.CanCastle(K_WHITE)) {
    fen += 'K';
  }
  if (board.CanCastle(Q_WHITE)) {
    fen += 'Q';
  }
  if (board.CanCastle(K_BLACK)) {
    fen += 'k';
  }
  if (board.CanCastle(Q_BLACK)) {
    fen += 'q';
  }

  if (fen.ends_with(' ')) {
    fen += '-';
  }

  if (board.EnPassantSquare()) {
    Coord coord = CoordFromBitboard(board.EnPassantSquare());
    char rank = '0' + coord.rank;

    fen += {' ', coord.file, rank, ' '};
  } else {
    fen += {' ', '-', ' '};
  }

  return fen += std::to_string(board.halfmove_clock_) + ' ' +
                std::to_string(board.fullmove_counter_);
}
