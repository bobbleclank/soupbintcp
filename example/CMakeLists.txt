add_subdirectory(utility)
add_subdirectory(test)

add_executable(bc_soup_client)
target_sources(bc_soup_client
  PRIVATE
    soup_client.cpp
)
target_include_directories(bc_soup_client PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(bc_soup_client PUBLIC bcutility bcsouptcp)

add_executable(bc_soup_server)
target_sources(bc_soup_server
  PRIVATE
    soup_server.cpp
)
target_include_directories(bc_soup_server PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(bc_soup_server PUBLIC bcutility bcsouptcp)
