add_subdirectory(utility)
add_subdirectory(test)

add_executable(bc_souptcp_client soup_client.cpp)
target_include_directories(bc_souptcp_client PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(bc_souptcp_client PUBLIC bcutility bcsouptcp)

add_executable(bc_souptcp_server soup_server.cpp)
target_include_directories(bc_souptcp_server PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(bc_souptcp_server PUBLIC bcutility bcsouptcp)
