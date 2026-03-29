#ifndef INCLUDE_BC_SOUP_SERVER_SERVER_H
#define INCLUDE_BC_SOUP_SERVER_SERVER_H

#include "bc/soup/expected.h"
#include "bc/soup/server/acceptor.h"

#include <asio.hpp>

#include <list>
#include <system_error>

namespace bc::soup::server {

class Acceptor_handler;

class Server {
public:
  explicit Server(asio::any_io_executor);

  [[nodiscard]] expected<Acceptor*, std::error_code>
  add_acceptor(const asio::ip::tcp::endpoint&);

  [[nodiscard]] expected<Acceptor*, std::error_code>
  add_acceptor(const asio::ip::tcp::endpoint&, Acceptor_handler&);

  void start();
  void stop();

private:
  asio::any_io_executor io_executor_;
  std::list<Acceptor> acceptors_;
  bool started_ = false;

  [[nodiscard]] expected<Acceptor*, std::error_code>
  add_acceptor(const asio::ip::tcp::endpoint&, Acceptor_handler*);
};

} // namespace bc::soup::server

#endif
