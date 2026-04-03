#ifndef INCLUDE_BC_SOUP_CLIENT_CLIENT_H
#define INCLUDE_BC_SOUP_CLIENT_CLIENT_H

#include "bc/soup/client/connection.h"
#include "bc/soup/expected.h"

#include <asio.hpp>

#include <cstddef>
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
  void set_write_packets_limit(std::size_t);

  [[nodiscard]] expected<Connection*, std::error_code>
  add_connection(const asio::ip::tcp::endpoint&);

  [[nodiscard]] expected<Connection*, std::error_code>
  add_connection(const asio::ip::tcp::endpoint&, Connection_handler&);

  [[nodiscard]] std::error_code start();
  void stop();

private:
  static constexpr std::size_t default_write_packets_limit = 100;

  Client_handler* handler_ = nullptr;
  asio::any_io_executor io_executor_;
  std::size_t write_packets_limit_ = default_write_packets_limit;
  std::list<Connection> connections_;
  bool started_ = false;

  [[nodiscard]] expected<Connection*, std::error_code>
  add_connection(const asio::ip::tcp::endpoint&, Connection_handler*);

  // Called by Connection
  friend class Connection;
  std::size_t write_packets_limit() const { return write_packets_limit_; }
};

} // namespace bc::soup::client

#endif
