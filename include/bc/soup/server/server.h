#ifndef INCLUDE_BC_SOUP_SERVER_SERVER_H
#define INCLUDE_BC_SOUP_SERVER_SERVER_H

#include <asio.hpp>

namespace bc::soup::server {

class Server {
public:
  explicit Server(asio::any_io_executor);

  void start();
  void stop();

private:
};

} // namespace bc::soup::server

#endif
