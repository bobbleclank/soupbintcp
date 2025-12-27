cmake_minimum_required(VERSION 3.23)

project(bc_soupbintcp VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

configure_file(${PROJECT_NAME}_config.h.in ${PROJECT_NAME}_config.h)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(test)
add_subdirectory(example)

install(
  TARGETS
    bcsouptcp
    bc_soup_client
    bc_soup_server
  FILE_SET HEADERS
)
