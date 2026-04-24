#include "bc/soup/client/tcp_connection.h"

#include "bc/soup/client/connection.h"
#include "bc/soup/client/handler.h"
#include "bc/soup/logical_packets.h"
#include "bc/soup/rw_packets.h"

#include <utility>

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
    handle_connect_failure(ec, "open");
    return;
  }
  if (const auto ec = socket_.set_no_delay()) {
    handle_connect_failure(ec, "set no delay");
    return;
  }
  handler_->connecting(connection_->endpoint());
  socket_.async_connect(connection_->endpoint());
}

void Tcp_connection::connect_failure(asio::error_code ec) {
  handle_connect_failure(ec, "connect");
}

void Tcp_connection::connect_success() {
  state_ = State::connected;
  const auto local_endpoint = socket_.local_endpoint();
  const auto remote_endpoint = socket_.remote_endpoint();
  handler_->connect_success(local_endpoint, remote_endpoint);

  socket_.async_read();

  Login_request_packet request;
  connection_->on_connect_success(request);
  handler_->logging_in(request);
  Write_packet packet(request.packet_type, request.payload_size);
  write(request, packet.payload_data());
  // Discard write failure: should not fail since first packet sent
  (void)socket_.async_write(std::move(packet));
}

void Tcp_connection::read_failure(asio::error_code) {
  terminate(Disconnect_reason::transport_error);
}

void Tcp_connection::read_failure(Packet_error) {
  terminate(Disconnect_reason::protocol_violation);
}

void Tcp_connection::read_success(const Read_packet&) {
}

void Tcp_connection::read_aborted() {
  terminate(Disconnect_reason::user_initiated);
}

void Tcp_connection::read_end_of_file() {
  terminate(Disconnect_reason::peer_closed);
}

void Tcp_connection::write_failure(asio::error_code) {
}

void Tcp_connection::write_success(const Write_packet&) {
}

void Tcp_connection::write_buffer_empty() {
}

void Tcp_connection::handle_connect_failure(asio::error_code ec,
                                            const char* phase) {
  state_ = State::disconnected;
  socket_.close();
  handler_->connect_failure(ec, phase);
  connection_->on_connect_failure();
}

void Tcp_connection::terminate(Disconnect_reason reason) {
  if (state_ == State::disconnected)
    return;
  state_ = State::disconnected;
  socket_.close();
  handler_->disconnect(reason);
}

void Tcp_connection::close() {
  socket_.close();
}

} // namespace bc::soup::client
