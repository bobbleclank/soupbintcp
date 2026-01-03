cmake_minimum_required(VERSION 3.23)

project(bc_soupbintcp VERSION 0.1)

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
