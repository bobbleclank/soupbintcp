add_library(soup
  client/client.cpp
  logical_packets.cpp
  packing.cpp
  rw_packets.cpp
  server/server.cpp
  socket.cpp
  socket_acceptor.cpp
  types.cpp)
target_include_directories(soup PUBLIC "${CMAKE_SOURCE_DIR}/include")
