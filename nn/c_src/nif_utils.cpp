#include <cstddef>
#include <cstdio>
#include <cstring>

#include <erl_nif.h>

#include "nn/nif_utils.hpp"

namespace erlang {

ERL_NIF_TERM atom(ErlNifEnv *env, char const *value) {
  ERL_NIF_TERM term;

  return enif_make_existing_atom(env, value, &term, ERL_NIF_LATIN1)
             ? term
             : enif_make_atom(env, value);
}

ERL_NIF_TERM binary(ErlNifEnv *env, char const *c_string) {
  ERL_NIF_TERM term;
  unsigned char *ptr;
  std::size_t len = std::strlen(c_string);

  if ((ptr = enif_make_new_binary(env, len, &term)) != nullptr) {
    std::strcpy((char *)ptr, c_string);

    return term;
  } else {
    enif_fprintf(
        stderr, "internal error: cannot allocate memory for binary string\r\n");

    return error(env);
  }
}

}  // namespace erlang
