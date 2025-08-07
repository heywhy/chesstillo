#ifndef NN_NIF_UTILS_HPP
#define NN_NIF_UTILS_HPP

#include <erl_nif.h>

namespace erlang {

ERL_NIF_TERM atom(ErlNifEnv *env, char const *msg);

}  // namespace erlang

#endif
