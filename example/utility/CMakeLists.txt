add_library(bcsouputility)
target_sources(bcsouputility
  PRIVATE
    io_context_runner.cpp
    option_convert.cpp
    option_error.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      io_context_runner.h
      option_convert.h
      option_error.h
)
target_compile_features(bcsouputility
  PRIVATE
    cxx_std_23
)
target_compile_options(bcsouputility
  PRIVATE
    -Wall
    -Wextra
    -pedantic
    -Werror
)
