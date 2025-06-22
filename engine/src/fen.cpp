#include <cstdint>
#include <format>
#include <string>

#include <engine/board.hpp>
#include <engine/position.hpp>
#include <engine/square.hpp>
#include <engine/types.hpp>

namespace engine {

Position Position::FromFen(const std::string_view fen) {
  Position position;
  Board &board = position.board_;

  PieceList &black_pieces = board.pieces[BLACK];
  PieceList &white_pieces = board.pieces[WHITE];

  std::uint_fast8_t rank = 7;
  std::uint_fast8_t file = 0;
  std::uint_fast8_t spaces = 0;
  std::uint16_t move_count = 0;
  std::uint_fast8_t en_passant_rank;
  char en_passant_file;

  for (const char c : fen) {
    Bitboard *piece = nullptr;

    switch (c) {
      case 'r':
        piece = &black_pieces[ROOK];
        break;

      case 'n':
        piece = &black_pieces[KNIGHT];
        break;

      case 'b':
        piece = &black_pieces[BISHOP];
        position.turn_ = BLACK;
        en_passant_file = c;
        break;

      case 'q':
        piece = &black_pieces[QUEEN];
        if (spaces == 2) position.castling_rights_ |= CASTLE_LEFT(BLACK);
        break;

      case 'k':
        piece = &black_pieces[KING];
        if (spaces == 2) position.castling_rights_ |= CASTLE_RIGHT(BLACK);
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
        if (spaces == 2) position.castling_rights_ |= CASTLE_LEFT(WHITE);
        break;

      case 'K':
        piece = &white_pieces[KING];

        if (spaces == 2) position.castling_rights_ |= CASTLE_RIGHT(WHITE);
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
        en_passant_file = c;
        break;

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
        file += c - '0';
        en_passant_rank = c - '0';
        move_count = c - '0' + (move_count * 10);
        break;

      case '0':
      case '9':
        move_count = c - '0' + (move_count * 10);
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
      std::uint_fast8_t square = square::From(file, rank);
      *piece |= square::BB(square);

      file++;
    } else if (spaces == 3 && (en_passant_rank == 3 || en_passant_rank == 6) &&
               en_passant_file >= 'a' && en_passant_file <= 'h') {
      std::uint_fast8_t rank = en_passant_rank - 1;
      std::uint_fast8_t file = en_passant_file - 97;
      std::uint_fast8_t square = square::From(file, rank);

      position.en_passant_sq_ = square::BB(square);
    } else if (spaces == 4) {
      position.halfmove_clock_ = move_count;
    } else if (spaces == 5) {
      position.fullmove_counter_ = move_count;
    }
  }

  position.UpdateInternals();
  position.UpdateMailbox();

  return position;
}

std::string Position::ToFen() const {
  int spaces = 0;
  std::string fen;
  Bitboard occupied_sqs = board_.occupied_sqs;

  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      std::uint_fast8_t square = square::From(file, rank);
      Bitboard bb = square::BB(square);

      if (occupied_sqs & bb) {
        if (spaces > 0) {
          fen += ('0' + spaces);
          spaces = 0;
        }

        char piece;

        if (PieceAt(&piece, square)) {
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

    if (rank != 0) fen += '/';
  }

  if (turn_ == WHITE) {
    fen += {' ', 'w', ' '};
  } else {
    fen += {' ', 'b', ' '};
  }

  if (CanCastle(CASTLE_RIGHT_WHITE)) {
    fen += 'K';
  }
  if (CanCastle(CASTLE_LEFT_WHITE)) {
    fen += 'Q';
  }
  if (CanCastle(CASTLE_RIGHT_BLACK)) {
    fen += 'k';
  }
  if (CanCastle(CASTLE_LEFT_BLACK)) {
    fen += 'q';
  }

  if (fen.ends_with(' ')) {
    fen += '-';
  }

  Coord coord;
  std::uint_fast8_t en_passant_square = square::Index(en_passant_sq_);

  if (CoordForSquare(&coord, en_passant_square)) {
    char rank = '0' + coord.rank;

    fen += {' ', coord.file, rank, ' '};
  } else {
    fen += {' ', '-', ' '};
  }

  std::string moves_clock_and_counter =
      std::format("{} {}", halfmove_clock_, fullmove_counter_);

  return fen.append(moves_clock_and_counter);
}

}  // namespace engine
