#ifndef INCLUDE_BC_SOUP_CLIENT_CLIENT_H
#define INCLUDE_BC_SOUP_CLIENT_CLIENT_H

#include "bc/soup/client/connection.h"
#include "bc/soup/expected.h"

#include <asio.hpp>

#include <list>
#include <system_error>

namespace bc::soup::client {

class Client_handler;
class Connection_handler;

class Client {
public:
  explicit Client(asio::any_io_executor);
  Client(asio::any_io_executor, Client_handler&);

  void set_handler(Client_handler&);

  [[nodiscard]] expected<Connection*, std::error_code>
  add_connection(const asio::ip::tcp::endpoint&);

  [[nodiscard]] expected<Connection*, std::error_code>
  add_connection(const asio::ip::tcp::endpoint&, Connection_handler&);

  void start();
  void stop();

private:
  Client_handler* handler_ = nullptr;
  asio::any_io_executor io_executor_;
  std::list<Connection> connections_;
  bool started_ = false;

  [[nodiscard]] expected<Connection*, std::error_code>
  add_connection(const asio::ip::tcp::endpoint&, Connection_handler*);
};

} // namespace bc::soup::client

#endif
