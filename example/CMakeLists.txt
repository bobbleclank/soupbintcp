add_subdirectory(utility)
add_subdirectory(test)

add_executable(soup_client soup_client.cpp)
target_link_libraries(soup_client PUBLIC utility soup)

add_executable(soup_server soup_server.cpp)
target_link_libraries(soup_server PUBLIC utility soup)
