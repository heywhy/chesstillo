include_guard(GLOBAL)

# Copied from: https://github.com/okeuday/GEPD/blob/master/CMake/erlang/FindErlang.cmake
# - Find Erlang
# This module finds if Erlang is installed and determines where the
# include files and libraries are. This code sets the following
# variables:
#
#  ERLANG_RUNTIME    = the full path to the Erlang runtime
#  ERLANG_COMPILE    = the full path to the Erlang compiler
#  ERLANG_EI_PATH    = the full path to the Erlang erl_interface path
#  ERLANG_ERTS_PATH    = the full path to the Erlang erts path
#  ERLANG_EI_INCLUDE_PATH = /include appended to ERLANG_EI_PATH
#  ERLANG_EI_LIBRARY_PATH = /lib appended to ERLANG_EI_PATH
#  ERLANG_ERTS_INCLUDE_PATH = /include appended to ERLANG_ERTS_PATH
#  ERLANG_ERTS_LIBRARY_PATH = /lib appended to ERLANG_ERTS_PATH

find_program(ERLANG_RUNTIME erl REQUIRED)
find_program(ERLANG_COMPILE erlc REQUIRED)

execute_process(
  COMMAND erl -noshell -eval "io:format(\"~s\", [code:root_dir()])" -s erlang halt
  OUTPUT_VARIABLE ERLANG_OTP_ROOT_DIR
)

execute_process(
  COMMAND erl -noshell -eval "io:format(\"~s\", [code:lib_dir()])" -s erlang halt
  OUTPUT_VARIABLE ERLANG_OTP_LIB_DIR
)

MESSAGE(STATUS "Using OTP lib: ${ERLANG_OTP_LIB_DIR} - found")

execute_process(
  COMMAND erl -noshell -eval "io:format(\"~s\",[filename:basename(code:lib_dir('erl_interface'))])" -s erlang halt
  OUTPUT_VARIABLE ERLANG_EI_DIR
)

execute_process(
  COMMAND erl -noshell -eval "io:format(\"~s\",[filename:basename(code:lib_dir('erts'))])" -s erlang halt
  OUTPUT_VARIABLE ERLANG_ERTS_DIR
)

message(STATUS "Using erl_interface version: ${ERLANG_EI_DIR}")
message(STATUS "Using erts version: ${ERLANG_ERTS_DIR}")

set(ERLANG_EI_PATH ${ERLANG_OTP_LIB_DIR}/${ERLANG_EI_DIR})
set(ERLANG_EI_INCLUDE_PATH ${ERLANG_OTP_LIB_DIR}/${ERLANG_EI_DIR}/include)
set(ERLANG_EI_LIBRARY_PATH ${ERLANG_OTP_LIB_DIR}/${ERLANG_EI_DIR}/lib)

set(ERLANG_ERTS_PATH ${ERLANG_OTP_ROOT_DIR}/${ERLANG_ERTS_DIR})
set(ERLANG_ERTS_INCLUDE_PATH ${ERLANG_OTP_ROOT_DIR}/${ERLANG_ERTS_DIR}/include)
set(ERLANG_ERTS_LIBRARY_PATH ${ERLANG_OTP_ROOT_DIR}/${ERLANG_ERTS_DIR}/lib)
set(ERTS_INCLUDE_DIR ${ERLANG_ERTS_INCLUDE_PATH})
