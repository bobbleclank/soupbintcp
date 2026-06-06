#ifndef INCLUDE_BC_SOUP_CLIENT_TCP_CONNECTION_H
#define INCLUDE_BC_SOUP_CLIENT_TCP_CONNECTION_H

#include "bc/soup/connection_state.h"
#include "bc/soup/heartbeat_timer.h"
#include "bc/soup/login_timer.h"
#include "bc/soup/socket.h"
#include "bc/soup/types.h"

#include <asio.hpp>

#include <cstddef>
#include <string_view>

namespace bc::soup {
class Read_packet;
class Write_packet;
} // namespace bc::soup

namespace bc::soup::client {

class Connection;
class Connection_handler;

class Tcp_connection final : public Socket::Handler,
                             public Login_timer::Handler,
                             public Heartbeat_timer::Handler {
public:
  Tcp_connection(asio::any_io_executor, Connection&, Connection_handler&,
                 std::size_t);
  ~Tcp_connection() = default;

  Tcp_connection(const Tcp_connection&) = delete;
  Tcp_connection& operator=(const Tcp_connection&) = delete;

  Tcp_connection(Tcp_connection&&) = delete;
  Tcp_connection& operator=(Tcp_connection&&) = delete;

  void connect_failure(asio::error_code) override;
  void connect_success() override;

  void read_failure(asio::error_code) override;
  void read_failure(Packet_error) override;
  void read_success(const Read_packet&) override;
  void read_aborted() override;
  void read_end_of_file() override;

  void write_failure(asio::error_code) override;
  void write_success(const Write_packet&) override;
  void write_buffer_empty() override;

  void closed() override;

  void login_timer_error(const asio::system_error&) override;
  void login_timer_expired() override;
  void login_timer_stopped() override;

  void heartbeat_timer_error(const asio::system_error&) override;
  void heartbeat_send_due() override;
  void heartbeat_receive_timeout() override;
  void heartbeat_timer_stopped() override;

private:
  Connection* connection_ = nullptr;
  Connection_handler* handler_ = nullptr;
  Socket socket_;
  Connection_state state_;
  Login_timer login_timer_;
  Heartbeat_timer heartbeat_timer_;
  bool socket_closed_ = false;
  bool login_timer_stopped_ = true;
  bool heartbeat_timer_stopped_ = true;

  [[nodiscard]] Packet_error process_packet(const Read_packet&);

  void process_debug(const void*, std::size_t);

  [[nodiscard]] Packet_error process_login_accepted(const void*, std::size_t);
  [[nodiscard]] Packet_error process_login_rejected(const void*, std::size_t);
  [[nodiscard]] Packet_error process_sequenced_data(const void*, std::size_t);
  [[nodiscard]] Packet_error process_server_heartbeat(std::size_t);
  [[nodiscard]] Packet_error process_end_of_session(std::size_t);

  void handle_connect_failure(asio::error_code, const char*);
  void disconnect(Disconnect_reason = Disconnect_reason::unmanaged_abort);
  void maybe_signal_closed();

  // Called by Connection
  friend class Connection;
  [[nodiscard]] Write_error send_packet(Write_packet&&);
  [[nodiscard]] Write_error send_debug_packet(std::string_view);
  void close();
};

} // namespace bc::soup::client

#endif
