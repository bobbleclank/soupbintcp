#ifndef INCLUDE_BC_SOUP_CLIENT_HANDLER_H
#define INCLUDE_BC_SOUP_CLIENT_HANDLER_H

#include <asio.hpp>

namespace bc::soup::client {

class Client_handler {
public:
protected:
  Client_handler() = default;
  ~Client_handler() = default;

  Client_handler(const Client_handler&) = default;
  Client_handler& operator=(const Client_handler&) = default;

  Client_handler(Client_handler&&) = default;
  Client_handler& operator=(Client_handler&&) = default;
};

class Connection_handler {
public:
  virtual void connecting(const asio::ip::tcp::endpoint&) = 0;
  virtual void connect_failure(asio::error_code, const char*) = 0;
  virtual void connect_success(const asio::ip::tcp::endpoint&,
                               const asio::ip::tcp::endpoint&) = 0;

protected:
  Connection_handler() = default;
  ~Connection_handler() = default;

  Connection_handler(const Connection_handler&) = default;
  Connection_handler& operator=(const Connection_handler&) = default;

  Connection_handler(Connection_handler&&) = default;
  Connection_handler& operator=(Connection_handler&&) = default;
};

} // namespace bc::soup::client

#endif
