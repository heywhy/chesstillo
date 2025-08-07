#include <erl_nif.h>

#include "nn/nif_utils.hpp"

namespace erlang {

ERL_NIF_TERM atom(ErlNifEnv *env, char const *value) {
  ERL_NIF_TERM term;

  return enif_make_existing_atom(env, value, &term, ERL_NIF_LATIN1)
             ? term
             : enif_make_atom(env, value);
}

}  // namespace erlang
