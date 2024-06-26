#include <string>

#include "fen.hpp"

// TODO: process castling rights, en passant & clock moves
Board &ApplyFen(Board &board, const char *fen) {
  Color turn;
  Bitboard *piece;

  int rank = 7;
  int file = 0;

  board.Reset();

  while (*fen) {
    switch (*fen) {
    case 'r':
      piece = &board.b_pieces_[ROOK];
      break;

    case 'n':
      piece = &board.b_pieces_[KNIGHT];
      break;

    case 'b':
      piece = &board.b_pieces_[BISHOP];
      turn = BLACK;
      break;

    case 'q':
      piece = &board.b_pieces_[QUEEN];
      break;

    case 'k':
      piece = &board.b_pieces_[KING];
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
      break;

    case 'K':
      piece = &board.w_pieces_[KING];
      break;

    case 'P':
      piece = &board.w_pieces_[PAWN];
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
      file += atoi(fen);
      piece = NULL;
      break;

    case '/':
      rank--;
      file = 0;
      piece = NULL;
      break;

    case 'w':
      turn = WHITE;
      break;

    case ' ':
      rank = -1;
      break;

    default:
      piece = NULL;
    }

    if (rank >= 0 && piece) {
      *piece |= BitboardForSquare((File)file, rank);

      ++file;
    }

    ++fen;
  }

  board.ComputeOccupied();

  return board;
}

std::string PositionToFen(Board &board) {
  int spaces = 0;
  std::string fen;

  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      Bitboard square = BitboardForSquare((File)file, rank);

      if (board.occupied_sqs_ & square) {
        if (spaces > 0) {
          fen.append(std::to_string(spaces));
          spaces = 0;
        }

        fen.push_back(board.PieceAtSquare(square));
      } else {
        spaces++;
      }
    }

    if (spaces > 0) {
      fen.append(std::to_string(spaces));
    }

    fen.push_back('/');

    spaces = 0;
  }

  // TODO: record castling rights, en passant & clock moves

  return fen.substr(0, fen.size() - 1).append(" w KQkq - 0 1");
}
