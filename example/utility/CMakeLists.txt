add_library(bcutility)
target_sources(bcutility
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
target_compile_features(bcutility
  PRIVATE
    cxx_std_23
)
target_compile_options(bcutility
  PRIVATE
    -Wall
    -Wextra
    -pedantic
    -Werror
)
