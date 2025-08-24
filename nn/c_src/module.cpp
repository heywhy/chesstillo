#include <cassert>

#include <erl_nif.h>

#include "engine/types.hpp"

#include "nn/chess.hpp"
#include "nn/nif_utils.hpp"

static ERL_NIF_TERM Hello(ErlNifEnv *env, int argc, const ERL_NIF_TERM[]) {
  assert(argc == 0);

  return erlang::atom(env, "world");
}

static ERL_NIF_TERM Constants(ErlNifEnv *env, int argc, const ERL_NIF_TERM[]) {
  assert(argc == 0);

  auto map = enif_make_new_map(env);

  assert(enif_make_map_put(env, map, erlang::atom(env, "king"),
                           enif_make_int(env, engine::Piece::KING), &map));

  assert(enif_make_map_put(env, map, erlang::atom(env, "queen"),
                           enif_make_int(env, engine::Piece::QUEEN), &map));
  assert(enif_make_map_put(env, map, erlang::atom(env, "rook"),
                           enif_make_int(env, engine::Piece::ROOK), &map));
  assert(enif_make_map_put(env, map, erlang::atom(env, "knight"),
                           enif_make_int(env, engine::Piece::KNIGHT), &map));
  assert(enif_make_map_put(env, map, erlang::atom(env, "bishop"),
                           enif_make_int(env, engine::Piece::BISHOP), &map));
  assert(enif_make_map_put(env, map, erlang::atom(env, "pawn"),
                           enif_make_int(env, engine::Piece::PAWN), &map));

  assert(enif_make_map_put(env, map, erlang::atom(env, "white"),
                           enif_make_int(env, engine::Color::WHITE), &map));
  assert(enif_make_map_put(env, map, erlang::atom(env, "black"),
                           enif_make_int(env, engine::Color::BLACK), &map));

  return map;
}

static int Load(ErlNifEnv *env, void **, ERL_NIF_TERM) {
  nn::chess::Load(env);

  return 0;
}

static ErlNifFunc nif_funcs[] = {
    {"init_chessboard", 0, nn::chess::InitBoard, 0},
    {"init_syzygy_tb", 1, nn::chess::InitTablebase, 0},
    {"syzygy_probe_dtz", 1, nn::chess::ProbeDTZ, 0},
    {"chessboard_to_fen", 1, nn::chess::BoardToFen, 0},
    {"set_piece_on_board", 4, nn::chess::SetPieceAt, 0},
    {"set_board_turn", 2, nn::chess::SetTurn, 0},
    {"game_over?", 1, nn::chess::IsGameOver, 0},
    {"constants", 0, Constants, 0},
    {"hello", 0, Hello, 0},
};

ERL_NIF_INIT(Elixir.NN, nif_funcs, Load, NULL, NULL, NULL);
