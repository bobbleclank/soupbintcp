add_library(soup
  client/client.cpp
  logical_packets.cpp
  packing.cpp
  rw_packets.cpp
  server/server.cpp)
target_include_directories(soup PUBLIC "${CMAKE_SOURCE_DIR}/include")
