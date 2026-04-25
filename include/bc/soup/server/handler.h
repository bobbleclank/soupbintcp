#ifndef INCLUDE_BC_SOUP_SERVER_HANDLER_H
#define INCLUDE_BC_SOUP_SERVER_HANDLER_H

#include "bc/soup/types.h"

#include <asio.hpp>

namespace bc::soup {
struct Login_accepted_packet;
struct Login_request_packet;
} // namespace bc::soup

namespace bc::soup::server {

class Acceptor_handler {
public:
  virtual void listening_setup_failure(asio::error_code, const char*) = 0;
  virtual void listening_setup_success(const asio::ip::tcp::endpoint&) = 0;

  virtual void accept_failure(asio::error_code) = 0;
  virtual void accept_success(const asio::ip::tcp::endpoint&,
                              const asio::ip::tcp::endpoint&) = 0;

  virtual void login_request(const Login_request_packet&) = 0;
  virtual void login_failure(Login_reject_reason) = 0;

  virtual void disconnect(Disconnect_reason) = 0;

protected:
  Acceptor_handler() = default;
  ~Acceptor_handler() = default;

  Acceptor_handler(const Acceptor_handler&) = default;
  Acceptor_handler& operator=(const Acceptor_handler&) = default;

  Acceptor_handler(Acceptor_handler&&) = default;
  Acceptor_handler& operator=(Acceptor_handler&&) = default;
};

class Port_handler {
public:
  virtual void login_failure(Login_reject_reason) = 0;
  virtual void login_success(const Login_accepted_packet&) = 0;

protected:
  Port_handler() = default;
  ~Port_handler() = default;

  Port_handler(const Port_handler&) = default;
  Port_handler& operator=(const Port_handler&) = default;

  Port_handler(Port_handler&&) = default;
  Port_handler& operator=(Port_handler&&) = default;
};

} // namespace bc::soup::server

#endif
