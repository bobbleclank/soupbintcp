#include "bc/soup/client/tcp_connection.h"

#include "bc/soup/client/connection.h"
#include "bc/soup/client/handler.h"

namespace bc::soup::client {

Tcp_connection::Tcp_connection(asio::any_io_executor io_executor,
                               Connection& connection,
                               Connection_handler& handler,
                               std::size_t write_packets_limit)
    : connection_(&connection),
      handler_(&handler),
      socket_(io_executor, *this) {

  socket_.set_write_packets_limit(write_packets_limit);
  if (const auto ec = socket_.open()) {
    connection_failure(ec, "open");
    return;
  }
  if (const auto ec = socket_.set_no_delay()) {
    connection_failure(ec, "set no delay");
    return;
  }
  handler_->connecting(connection_->endpoint());
  socket_.async_connect(connection_->endpoint());
}

void Tcp_connection::connect_failure(asio::error_code ec) {
  connection_failure(ec, "connect");
}

void Tcp_connection::connect_success() {
  state_ = State::connected;
  const auto local_endpoint = socket_.local_endpoint();
  const auto remote_endpoint = socket_.remote_endpoint();
  handler_->connection_success(local_endpoint, remote_endpoint);

  socket_.async_read();
}

void Tcp_connection::read_failure(asio::error_code) {
}

void Tcp_connection::read_failure(Packet_error) {
}

void Tcp_connection::read_success(const Read_packet&) {
}

void Tcp_connection::read_end_of_file() {
}

void Tcp_connection::write_failure(asio::error_code) {
}

void Tcp_connection::write_success(const Write_packet&) {
}

void Tcp_connection::write_buffer_empty() {
}

void Tcp_connection::connection_failure(asio::error_code ec,
                                        const char* phase) {
  socket_.close();
  state_ = State::disconnected;
  handler_->connection_failure(ec, phase);
}

void Tcp_connection::close() {
  socket_.close();
  state_ = State::disconnected;
}

} // namespace bc::soup::client
