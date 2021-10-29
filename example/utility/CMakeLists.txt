add_library(utility
  io_context_runner.cpp
  option_convert.cpp
  option_error.cpp)
target_include_directories(utility PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
