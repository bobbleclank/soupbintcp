cmake_minimum_required(VERSION 3.20)

project(bc_soupbintcp VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

configure_file(${PROJECT_NAME}_config.h.in ${PROJECT_NAME}_config.h)

include_directories(PUBLIC "${CMAKE_BINARY_DIR}")

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(example)
