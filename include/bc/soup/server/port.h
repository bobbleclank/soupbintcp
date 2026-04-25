#ifndef INCLUDE_BC_SOUP_SERVER_PORT_H
#define INCLUDE_BC_SOUP_SERVER_PORT_H

#include "bc/soup/expected.h"

#include <string>
#include <string_view>

namespace bc::soup {
struct Login_accepted_packet;
struct Login_rejected_packet;
struct Login_request_packet;
} // namespace bc::soup

namespace bc::soup::server {

class Port_handler;
class Tcp_connection;

class Port {
public:
  Port(std::string_view, std::string_view, Port_handler*);

  void set_handler(Port_handler&);

  std::string_view username() const { return username_; }
  std::string_view password() const { return password_; }

private:
  Port_handler* handler_ = nullptr;
  std::string username_;
  std::string password_;
  Tcp_connection* connection_ = nullptr;

  // Called by Acceptor
  friend class Acceptor;
  bool is_handler_set() const;
  [[nodiscard]] expected<Login_accepted_packet, Login_rejected_packet>
  on_login_request(Tcp_connection&, const Login_request_packet&,
                   Port_handler*&);
};

} // namespace bc::soup::server

#endif
