include_guard(GLOBAL)

include(FetchContent)

FetchContent_Declare(
  magic_bits
  GIT_REPOSITORY https://github.com/goutham/magic-bits.git
  GIT_TAG 3152d1bf63d7ec2ce3b69dc5b2474db7abd419fa
  GIT_SHALLOW true
)

FetchContent_MakeAvailable(magic_bits)

add_library(magic_bits INTERFACE)

target_include_directories(
  magic_bits
  INTERFACE ${magic_bits_SOURCE_DIR}/include
)
