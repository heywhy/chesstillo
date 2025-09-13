#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include <erl_nif.h>

#include "engine/constants.hpp"
#include "engine/move_gen.hpp"
#include "engine/position.hpp"
#include "engine/types.hpp"
#include "tb/tbprobe.h"

#include "nn/chess.hpp"
#include "nn/nif_utils.hpp"

namespace nn {
namespace chess {

ErlNifResourceType *kResourceType;

void Destroy(ErlNifEnv *, void *ptr) {
  auto resource = reinterpret_cast<engine::Position **>(ptr);

  delete *resource;
}

void Load(ErlNifEnv *env) {
  kResourceType = enif_open_resource_type(env, NULL, "position_store", Destroy,
                                          ERL_NIF_RT_CREATE, NULL);
}

ERL_NIF_TERM InitBoard(ErlNifEnv *env, int argc, const ERL_NIF_TERM[]) {
  assert(argc == 0);

  auto resource = erlang::alloc<engine::Position *>(kResourceType);

  auto position = new engine::Position();

  position->Reset();

  *resource = position;

  auto term = erlang::resource(env, resource);

  erlang::release(resource);

  return term;
}

ERL_NIF_TERM SetTurn(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  assert(argc == 2);

  int color;
  engine::Position **position;

  if (!erlang::resource(env, argv[0], kResourceType, &position) ||
      !enif_get_int(env, argv[1], &color)) {
    return enif_make_badarg(env);
  }

  (*position)->SetTurn(static_cast<engine::Color>(color));

  return erlang::ok(env);
}

ERL_NIF_TERM SetPieceAt(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  assert(argc == 4);

  int piece, color, square;
  engine::Position **position;

  if (!erlang::resource(env, argv[0], kResourceType, &position) ||
      !enif_get_int(env, argv[1], &piece) ||
      !enif_get_int(env, argv[2], &color) ||
      !enif_get_int(env, argv[3], &square)) {
    return enif_make_badarg(env);
  }

  (*position)->SetPieceAt(static_cast<engine::Color>(color),
                          static_cast<engine::Piece>(piece), square);

  return erlang::ok(env);
}

ERL_NIF_TERM BoardToFen(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  assert(argc == 1);

  engine::Position **position;

  if (!erlang::resource(env, argv[0], kResourceType, &position)) {
    return enif_make_badarg(env);
  }

  auto fen = (*position)->ToFen();

  return erlang::binary(env, fen);
}

ERL_NIF_TERM InitTablebase(ErlNifEnv *env, int, const ERL_NIF_TERM argv[]) {
  unsigned int len;
  auto path = argv[0];

  if (!enif_get_string_length(env, path, &len, ERL_NIF_UTF8)) {
    return enif_make_badarg(env);
  }

  ++len;
  auto str = new char[len];

  if (enif_get_string(env, path, str, len, ERL_NIF_UTF8) !=
      static_cast<int>(len)) {
    return enif_make_badarg(env);
  }

  init_tablebases(str);

  delete[] str;

  return erlang::ok(env);
}

ERL_NIF_TERM ProbeDTZ(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  assert(argc == 1);

  int value, success;
  engine::Position **position;

  if (!erlang::resource(env, argv[0], kResourceType, &position)) {
    return enif_make_badarg(env);
  }

  value = probe_dtz(**position, &success);

  if (!success) {
    return erlang::error(env);
  }

  return enif_make_int(env, value);
}

bool SufficientMaterials(const engine::PieceList &pieces);

ERL_NIF_TERM IsGameOver(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  assert(argc == 1);
  engine::Position **position;

  if (!erlang::resource(env, argv[0], kResourceType, &position)) {
    return enif_make_badarg(env);
  }

  auto pos = *position;
  const auto b_pieces = pos->Pieces(engine::BLACK);
  const auto w_pieces = pos->Pieces(engine::WHITE);

  auto b_king = b_pieces[engine::KING];
  auto w_king = w_pieces[engine::KING];

  if ((KING_ATTACKS(w_king) & b_king) || (KING_ATTACKS(b_king) & w_king)) {
    return erlang::atom(env, "true");
  }

  auto moves = pos->LegalMoves();

  if (moves.empty()) {
    return erlang::atom(env, "true");
  }

  if (SufficientMaterials(w_pieces) || SufficientMaterials(b_pieces)) {
    return erlang::atom(env, "false");
  }

  return erlang::atom(env, "true");
}

bool SufficientMaterials(const engine::PieceList &pieces) {
  auto &pawns = pieces[engine::PAWN];
  auto &bishops = pieces[engine::BISHOP];
  auto &knights = pieces[engine::KNIGHT];
  auto rooks_and_queens = pieces[engine::ROOK] | pieces[engine::QUEEN];

  if (pawns || rooks_and_queens || (knights && bishops) ||
      std::popcount(knights) >= 2) {
    return true;
  }

  if (bishops & engine::kDarkSquares && bishops & engine::kLightSquares) {
    return true;
  }

  return false;
}

}  // namespace chess

}  // namespace nn
