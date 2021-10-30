add_subdirectory(utility)
add_subdirectory(test)

add_executable(bc_soup_client soup_client.cpp)
target_link_libraries(bc_soup_client PUBLIC utility bcsoup)

add_executable(bc_soup_server soup_server.cpp)
target_link_libraries(bc_soup_server PUBLIC utility bcsoup)
