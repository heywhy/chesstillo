include(find_erlang)
include(format)

file(GLOB_RECURSE NN_SRC LIST_DIRECTORIES false **/*.cpp **/*.hpp)

format(nn "${NN_SRC}")

add_library(nn
  SHARED
  c_src/nif_utils.cpp
  c_src/chess.cpp
  c_src/module.cpp
)

if(TARGET nn_format)
  add_dependencies(nn nn_format)
endif()

set_target_properties(nn PROPERTIES SUFFIX .so)

target_include_directories(nn PRIVATE include)
target_include_directories(nn SYSTEM PRIVATE ${ERTS_INCLUDE_DIR})

target_compile_options(nn PRIVATE ${COMPILE_OPTIONS})

target_link_options(nn PUBLIC -fPIC ${LINK_OPTIONS})

if (APPLE)
  target_link_options(nn PUBLIC -undefined dynamic_lookup -flat_namespace)
endif()

target_link_libraries(nn
  PRIVATE engine
  PRIVATE uci
)

add_custom_command(
  TARGET nn
  POST_BUILD
  COMMAND cp $<TARGET_FILE:nn> ${chesstillo_SOURCE_DIR}/nn/priv
)
