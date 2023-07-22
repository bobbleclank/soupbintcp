#ifndef INCLUDE_BC_SOUP_CLIENT_CLIENT_H
#define INCLUDE_BC_SOUP_CLIENT_CLIENT_H

#include <asio.hpp>

namespace bc::soup::client {

class Client {
public:
  explicit Client(asio::any_io_executor);

  void start();
  void stop();

private:
};

} // namespace bc::soup::client

#endif
