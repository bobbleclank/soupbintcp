#ifndef INCLUDE_BC_SOUP_SERVER_TCP_CONNECTION_H
#define INCLUDE_BC_SOUP_SERVER_TCP_CONNECTION_H

#include "bc/soup/connection_state.h"
#include "bc/soup/socket.h"
#include "bc/soup/types.h"

#include <asio.hpp>

#include <cstddef>

namespace bc::soup {
class Read_packet;
class Write_packet;
} // namespace bc::soup

namespace bc::soup::server {

class Acceptor;
class Port;
class Port_handler;

class Tcp_connection final : public Socket::Handler {
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

private:
  Acceptor* acceptor_ = nullptr;
  Port* port_ = nullptr;
  Port_handler* handler_ = nullptr;
  Socket socket_;
  Connection_state state_;

  void process_packet(const Read_packet&);

  [[nodiscard]] Packet_error process_login_request(const void*, std::size_t);

  void terminate(Disconnect_reason = Disconnect_reason::unmanaged_abort);
  void initiate_disconnect(Disconnect_reason, bool = false);

  // Called by Acceptor
  friend class Acceptor;
  void close();

  // Called by Port
  friend class Port;
  void send_end_of_session();
};

} // namespace bc::soup::server

#endif
