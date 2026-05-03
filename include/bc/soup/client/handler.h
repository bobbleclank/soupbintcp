#ifndef INCLUDE_BC_SOUP_CLIENT_HANDLER_H
#define INCLUDE_BC_SOUP_CLIENT_HANDLER_H

#include "bc/soup/types.h"

#include <asio.hpp>

namespace bc::soup {
struct Login_accepted_packet;
struct Login_request_packet;
} // namespace bc::soup

namespace bc::soup::client {

class Client_handler {
public:
  virtual void end_of_session() = 0;

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

  virtual void logging_in(const Login_request_packet&) = 0;
  virtual void login_failure(Login_reject_reason) = 0;
  virtual void login_success(const Login_accepted_packet&) = 0;

  virtual void disconnect(Disconnect_reason) = 0;

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
