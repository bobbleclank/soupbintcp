add_library(bcsoup
  client/client.cpp
  client/connection.cpp
  client/tcp_connection.cpp
  file_store.cpp
  logical_packets.cpp
  packing.cpp
  rw_packets.cpp
  server/acceptor.cpp
  server/port.cpp
  server/server.cpp
  server/tcp_connection.cpp
  socket.cpp
  socket_acceptor.cpp
  types.cpp
  validate.cpp)
target_include_directories(bcsoup PUBLIC "${CMAKE_SOURCE_DIR}/include")
