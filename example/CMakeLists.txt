add_subdirectory(utility)
add_subdirectory(test)

add_executable(bc_soup_client soup_client.cpp)
target_include_directories(bc_soup_client PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(bc_soup_client PUBLIC bcutility bcsouptcp)

add_executable(bc_soup_server soup_server.cpp)
target_include_directories(bc_soup_server PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(bc_soup_server PUBLIC bcutility bcsouptcp)
