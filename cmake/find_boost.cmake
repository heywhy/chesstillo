include_guard(GLOBAL)

FetchContent_Declare(
  boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.88.0
  GIT_SHALLOW true
)

set(BOOST_RUNTIME_LINK static)
list(APPEND BOOST_INCLUDE_LIBRARIES asio filesystem process)

FetchContent_MakeAvailable(boost)

target_compile_definitions(boost_asio
  INTERFACE
  BOOST_ASIO_NO_DEPRECATED
  BOOST_ASIO_NO_DYNAMIC_BUFFER_V1
  BOOST_ASIO_DISABLE_THREADS
)
