#ifndef INCLUDE_BC_SOUP_CLIENT_CONNECTION_H
#define INCLUDE_BC_SOUP_CLIENT_CONNECTION_H

#include "bc/soup/client/tcp_connection.h"

#include <asio.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace bc::soup {
struct Login_accepted_packet;
struct Login_request_packet;
} // namespace bc::soup

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
  [[nodiscard]] std::error_code set_session(std::string_view);

  const asio::ip::tcp::endpoint& endpoint() const { return endpoint_; }

  std::string_view username() const { return username_; }
  std::string_view password() const { return password_; }
  std::string_view session() const { return session_; }

private:
  Client* client_ = nullptr;
  Connection_handler* handler_ = nullptr;
  asio::any_io_executor io_executor_;
  asio::ip::tcp::endpoint endpoint_;
  std::string username_;
  std::string password_;
  std::string session_;
  std::optional<Tcp_connection> connection_;

  // Called by Client
  friend class Client;
  bool is_handler_set() const;
  void connect();
  void disconnect();

  // Called by Tcp_connection
  friend class Tcp_connection;
  void on_connect_failure();
  void on_connect_success(Login_request_packet&);
  void on_login_success(const Login_accepted_packet&);
};

} // namespace bc::soup::client

#endif
