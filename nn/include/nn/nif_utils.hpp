#ifndef NN_NIF_UTILS_HPP
#define NN_NIF_UTILS_HPP

#include <string>

#include <erl_nif.h>

namespace erlang {

ERL_NIF_TERM atom(ErlNifEnv *env, char const *msg);

inline ERL_NIF_TERM ok(ErlNifEnv *env) { return atom(env, "ok"); }

inline ERL_NIF_TERM error(ErlNifEnv *env) { return atom(env, "error"); }

ERL_NIF_TERM binary(ErlNifEnv *env, char const *c_string);

inline ERL_NIF_TERM binary(ErlNifEnv *env, const std::string &str) {
  return binary(env, str.c_str());
}

template <typename T>
bool resource(ErlNifEnv *env, ERL_NIF_TERM term, ErlNifResourceType *type,
              T result) {
  return enif_get_resource(env, term, type, (void **)result);
}

template <typename T>
ERL_NIF_TERM resource(ErlNifEnv *env, T *a) {
  return enif_make_resource(env, (void *)a);
}

template <typename T>
T *alloc(ErlNifResourceType *type) {
  return (T *)enif_alloc_resource(type, sizeof(T));
}

template <typename T>
void release(T *object) {
  enif_release_resource((void *)object);
}

}  // namespace erlang

#endif
