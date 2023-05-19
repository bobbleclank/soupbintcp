#ifndef INCLUDE_BC_SOUP_CLIENT_TCP_CONNECTION_H
#define INCLUDE_BC_SOUP_CLIENT_TCP_CONNECTION_H

#include "bc/soup/socket.h"

namespace bc::soup::client {

class Tcp_connection final : public Socket::Handler {
public:
  Tcp_connection();
  ~Tcp_connection() = default;

  Tcp_connection(const Tcp_connection&) = delete;
  Tcp_connection& operator=(const Tcp_connection&) = delete;

  Tcp_connection(Tcp_connection&&) = delete;
  Tcp_connection& operator=(Tcp_connection&&) = delete;

  void connect_failure(boost::system::error_code) override;
  void connect_success() override;

  void read_failure(boost::system::error_code) override;
  void read_failure(Packet_error) override;
  void read_completed(const Read_packet&) override;
  void end_of_file() override;

  void write_failure(boost::system::error_code) override;
  void write_completed(const Write_packet&) override;
  void write_buffer_empty() override;

private:
};

} // namespace bc::soup::client

#endif
