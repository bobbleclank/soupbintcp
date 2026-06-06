#ifndef INCLUDE_BC_SOUP_SERVER_TCP_CONNECTION_H
#define INCLUDE_BC_SOUP_SERVER_TCP_CONNECTION_H

#include "bc/soup/connection_state.h"
#include "bc/soup/heartbeat_timer.h"
#include "bc/soup/socket.h"
#include "bc/soup/types.h"

#include <asio.hpp>

#include <cstddef>
#include <string_view>

namespace bc::soup {
class Read_packet;
class Write_packet;
} // namespace bc::soup

namespace bc::soup::server {

class Acceptor;
class Port;
class Port_handler;

class Tcp_connection final : public Socket::Handler,
                             public Heartbeat_timer::Handler {
public:
  Tcp_connection(asio::any_io_executor, Socket&&, Acceptor&);
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

  void heartbeat_timer_error(const asio::system_error&) override;
  void heartbeat_send_due() override;
  void heartbeat_receive_timeout() override;
  void heartbeat_timer_stopped() override;

private:
  Acceptor* acceptor_ = nullptr;
  Port* port_ = nullptr;
  Port_handler* handler_ = nullptr;
  Socket socket_;
  Connection_state state_{Connection_state::State::connected};
  Heartbeat_timer heartbeat_timer_;
  bool socket_closed_ = false;
  bool heartbeat_timer_stopped_ = true;

  [[nodiscard]] Packet_error process_packet(const Read_packet&);

  void process_debug(const void*, std::size_t);

  [[nodiscard]] Packet_error process_login_request(const void*, std::size_t);
  [[nodiscard]] Packet_error process_unsequenced_data(const void*, std::size_t);
  [[nodiscard]] Packet_error process_client_heartbeat(std::size_t);
  [[nodiscard]] Packet_error process_logout_request(std::size_t);

  void disconnect(Disconnect_reason = Disconnect_reason::unmanaged_abort);
  void prepare_graceful_disconnect(Disconnect_reason);
  void maybe_signal_closed();

  // Called by Acceptor
  friend class Acceptor;
  void close();

  // Called by Port
  friend class Port;
  [[nodiscard]] Write_error send_packet(Write_packet&&);
  [[nodiscard]] Write_error send_debug_packet(std::string_view);
};

} // namespace bc::soup::server

#endif
