add_library(utility
  option_convert.cpp
  option_error.cpp)
target_include_directories(utility PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
