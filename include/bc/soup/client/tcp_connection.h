#ifndef INCLUDE_BC_SOUP_CLIENT_TCP_CONNECTION_H
#define INCLUDE_BC_SOUP_CLIENT_TCP_CONNECTION_H

#include "bc/soup/socket.h"

#include <asio.hpp>

#include <cstddef>

namespace bc::soup::client {

class Connection;
class Connection_handler;

class Tcp_connection final : public Socket::Handler {
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

private:
  enum class State {
    connecting,
    connected,
    disconnected
  };

  Connection* connection_ = nullptr;
  Connection_handler* handler_ = nullptr;
  Socket socket_;
  State state_ = State::connecting;

  void handle_connect_failure(asio::error_code, const char*);

  // Called by Connection
  friend class Connection;
  void close();
};

} // namespace bc::soup::client

#endif
