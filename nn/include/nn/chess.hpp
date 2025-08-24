#ifndef NN_CHESS_HPP
#define NN_CHESS_HPP

#include <erl_nif.h>

namespace nn {
namespace chess {

void Load(ErlNifEnv *env);

ERL_NIF_TERM InitBoard(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM BoardToFen(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM SetTurn(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM SetPieceAt(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM IsGameOver(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM InitTablebase(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM ProbeDTZ(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);

}  // namespace chess
}  // namespace nn

#endif
