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
target_link_libraries(bcsouputility
  PRIVATE
    bcsoup_warnings
)
target_compile_features(bcsouputility
  PUBLIC
    cxx_std_23
)
