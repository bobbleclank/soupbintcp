#ifndef INCLUDE_BC_SOUP_CLIENT_CONNECTION_H
#define INCLUDE_BC_SOUP_CLIENT_CONNECTION_H

#include "bc/soup/client/tcp_connection.h"

#include <asio.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace bc::soup::client {

class Client;
class Connection_handler;

class Connection {
public:
  Connection(asio::any_io_executor, const asio::ip::tcp::endpoint&, Client&,
             Connection_handler*);

  void set_handler(Connection_handler&);

  [[nodiscard]] std::error_code set_username(std::string_view);
  [[nodiscard]] std::error_code set_password(std::string_view);

  const asio::ip::tcp::endpoint& endpoint() const { return endpoint_; }

  std::string_view username() const { return username_; }
  std::string_view password() const { return password_; }

private:
  Client* client_ = nullptr;
  Connection_handler* handler_ = nullptr;
  asio::any_io_executor io_executor_;
  asio::ip::tcp::endpoint endpoint_;
  std::string username_;
  std::string password_;
  std::optional<Tcp_connection> connection_;

  // Called by Client
  friend class Client;
  bool is_handler_set() const;
  void connect();
  void disconnect();
};

} // namespace bc::soup::client

#endif
