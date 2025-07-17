#include <cstdint>
#include <format>
#include <string>

#include <engine/board.hpp>
#include <engine/hash.hpp>
#include <engine/position.hpp>
#include <engine/square.hpp>
#include <engine/types.hpp>

namespace engine {

Position Position::FromFen(const std::string_view &fen) {
  Position position;

  ApplyFen(&position, fen);

  return position;
}

void Position::ApplyFen(Position *position, const std::string_view &fen) {
  position->Reset();

  Board &board = position->board_;

  PieceList &black_pieces = board.pieces[BLACK];
  PieceList &white_pieces = board.pieces[WHITE];

  int rank = 7;
  int file = 0;
  int spaces = 0;
  long move_count = 0;
  int en_passant_rank;
  char en_passant_file;

  for (const char c : fen) {
    Piece piece = NONE;
    Color color = WHITE;
    Bitboard *bb = nullptr;

    switch (c) {
      case 'r':
        piece = ROOK;
        color = BLACK;
        bb = &black_pieces[piece];
        break;

      case 'n':
        color = BLACK;
        piece = KNIGHT;
        bb = &black_pieces[piece];
        break;

      case 'b':
        color = BLACK;
        piece = BISHOP;
        bb = &black_pieces[piece];
        position->turn_ = BLACK;
        en_passant_file = c;
        break;

      case 'q':
        color = BLACK;
        piece = QUEEN;
        bb = &black_pieces[piece];
        if (spaces == 2)
          position->castling_rights_ |= position::CASTLE_B_QUEEN_SIDE;
        break;

      case 'k':
        piece = KING;
        color = BLACK;
        bb = &black_pieces[piece];
        if (spaces == 2)
          position->castling_rights_ |= position::CASTLE_B_KING_SIDE;
        break;

      case 'p':
        piece = PAWN;
        color = BLACK;
        bb = &black_pieces[piece];
        break;

      case 'R':
        piece = ROOK;
        color = WHITE;
        bb = &white_pieces[piece];
        break;

      case 'N':
        color = WHITE;
        piece = KNIGHT;
        bb = &white_pieces[piece];
        break;

      case 'B':
        color = WHITE;
        piece = BISHOP;
        bb = &white_pieces[piece];
        break;

      case 'Q':
        color = WHITE;
        piece = QUEEN;
        bb = &white_pieces[piece];
        if (spaces == 2)
          position->castling_rights_ |= position::CASTLE_W_QUEEN_SIDE;
        break;

      case 'K':
        piece = KING;
        color = WHITE;
        bb = &white_pieces[piece];

        if (spaces == 2)
          position->castling_rights_ |= position::CASTLE_W_KING_SIDE;
        break;

      case 'P':
        piece = PAWN;
        color = WHITE;
        bb = &white_pieces[piece];
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
        move_count = 0;
        break;

      case 'w':
        position->turn_ = WHITE;
        break;

      case ' ':
        spaces++;
        move_count = 0;
        en_passant_rank = 0;
        en_passant_file = 0;
        break;
    }

    if (spaces == 0 && bb && piece != NONE) {
      int square = square::From(file, rank);
      *bb |= square::BB(square);

      file++;
      position->hash_ ^= HASH1(square, color, piece);
    } else if (spaces == 3 && (en_passant_rank == 3 || en_passant_rank == 6) &&
               en_passant_file >= 'a' && en_passant_file <= 'h') {
      int rank = en_passant_rank - 1;
      int file = en_passant_file - 97;
      int square = square::From(file, rank);

      position->en_passant_sq_ = square::BB(square);
      position->hash_ ^= kZobrist.en_passant_file[file];
    } else if (spaces == 4) {
      position->halfmove_clock_ = move_count;
    } else if (spaces == 5) {
      position->fullmove_counter_ = move_count;
    }
  }

  if (position->turn_ == BLACK) {
    position->hash_ ^= kZobrist.color;
  }

  std::uint8_t copy = position->castling_rights_;
  int index = square::Index(copy);

  while (copy) {
    position->hash_ ^= kZobrist.castling_rights[index];

    copy ^= 1 << index;
    index = square::Index(copy);
  }

  position->UpdateInternals();
}

std::string Position::ToFen() const {
  int spaces = 0;
  std::string fen;
  Bitboard occupied_sqs = board_.occupied_sqs;

  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      int square = square::From(file, rank);
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

  if (castling_rights_ & position::CASTLE_W_KING_SIDE) {
    fen += 'K';
  }

  if (castling_rights_ & position::CASTLE_W_QUEEN_SIDE) {
    fen += 'Q';
  }

  if (castling_rights_ & position::CASTLE_B_KING_SIDE) {
    fen += 'k';
  }

  if (castling_rights_ & position::CASTLE_B_QUEEN_SIDE) {
    fen += 'q';
  }

  if (fen.ends_with(' ')) {
    fen += '-';
  }

  Coord coord;
  int en_passant_square = square::Index(en_passant_sq_);

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
