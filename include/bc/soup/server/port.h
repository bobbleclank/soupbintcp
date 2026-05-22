#ifndef INCLUDE_BC_SOUP_SERVER_PORT_H
#define INCLUDE_BC_SOUP_SERVER_PORT_H

#include "bc/soup/expected.h"
#include "bc/soup/types.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace bc::soup {
struct Login_accepted_packet;
struct Login_rejected_packet;
struct Login_request_packet;
class Write_packet;
} // namespace bc::soup

namespace bc::soup::server {

class Port_handler;
class Tcp_connection;
class Message;

class Port {
public:
  Port(std::string_view, std::string_view, Port_handler*);

  void set_handler(Port_handler&);

  void set_next_sequence_number(std::uint64_t);

  std::string_view username() const { return username_; }
  std::string_view password() const { return password_; }

  std::uint64_t next_sequence_number() const { return next_sequence_number_; }
  bool has_session_ended() const { return has_session_ended_; }

  [[nodiscard]] Write_error send_message(const void*, std::size_t);
  [[nodiscard]] Write_error send_message(Message&&);

private:
  Port_handler* handler_ = nullptr;
  std::string username_;
  std::string password_;
  std::uint64_t next_sequence_number_ = 1;
  bool has_session_ended_ = false;
  Tcp_connection* connection_ = nullptr;

  [[nodiscard]] Write_error send_packet(Write_packet&&);

  // Called by Acceptor
  friend class Acceptor;
  bool is_handler_set() const;
  [[nodiscard]] expected<Login_accepted_packet, Login_rejected_packet>
  on_login_request(Tcp_connection&, const Login_request_packet&,
                   std::string_view, Port_handler*&);
  void end_session();

  // Called by Tcp_connection
  friend class Tcp_connection;
  void on_unsequenced_data(const void*, std::size_t);
  void on_closed();
};

} // namespace bc::soup::server

#endif
