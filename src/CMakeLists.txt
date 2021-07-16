add_library(soup
  client/client.cpp
  server/server.cpp)
target_include_directories(soup PUBLIC "${CMAKE_SOURCE_DIR}/include")
