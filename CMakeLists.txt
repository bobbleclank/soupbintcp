cmake_minimum_required(VERSION 3.23)

project(bc_soupbintcp VERSION 0.1)

configure_file(
  "${PROJECT_NAME}_config.h.in"
  "${PROJECT_NAME}_config.h"
)

option(BCSOUP_BUILD_TESTS "Build the unit tests" ON)
option(BCSOUP_BUILD_EXAMPLES "Build the examples" ON)

add_subdirectory(src)
add_subdirectory(include)

if(BCSOUP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(BCSOUP_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

install(
  TARGETS
    bcsouptcp
  FILE_SET HEADERS
)
