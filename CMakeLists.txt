cmake_minimum_required(VERSION 3.20)

project(libsoup VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -pedantic -Werror)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

configure_file(libsoup_config.h.in libsoup_config.h)

include_directories(PUBLIC "${CMAKE_BINARY_DIR}")

add_subdirectory(src)
add_subdirectory(example)
