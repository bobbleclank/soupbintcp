add_library(bcutility)
target_sources(bcutility
  PRIVATE
    io_context_runner.cpp
    option_convert.cpp
    option_error.cpp
)
target_include_directories(bcutility PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
