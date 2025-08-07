#include <erl_nif.h>

#include "nn/nif_utils.hpp"

namespace nn {

static ERL_NIF_TERM hello(ErlNifEnv *env, int, const ERL_NIF_TERM[]) {
  return erlang::atom(env, "world");
}

static ErlNifFunc nif_funcs[] = {{"hello", 0, hello, 0}};

ERL_NIF_INIT(Elixir.NN, nif_funcs, NULL, NULL, NULL, NULL);

}  // namespace nn
