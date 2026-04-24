#include "bc/soup/server/tcp_connection.h"

#include "bc/soup/logical_packets.h"
#include "bc/soup/rw_packets.h"
#include "bc/soup/server/acceptor.h"

#include <cassert>
#include <utility>

namespace bc::soup::server {

Tcp_connection::Tcp_connection(asio::any_io_executor, Socket&& socket,
                               Acceptor& acceptor)
    : acceptor_(&acceptor), socket_(std::move(socket)) {

  socket_.set_handler(*this);
  socket_.async_read();
}

void Tcp_connection::connect_failure(asio::error_code) {
  assert(false);
}

void Tcp_connection::connect_success() {
  assert(false);
}

void Tcp_connection::read_failure(asio::error_code) {
  terminate(Disconnect_reason::transport_error);
}

void Tcp_connection::read_failure(Packet_error) {
  terminate(Disconnect_reason::protocol_violation);
}

void Tcp_connection::read_success(const Read_packet& packet) {
  process_packet(packet);
  socket_.async_read();
}

void Tcp_connection::read_aborted() {
  const auto reason = (pending_reason_ == Disconnect_reason::none)
                          ? Disconnect_reason::unmanaged_abort
                          : pending_reason_;
  terminate(reason);
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

void Tcp_connection::process_packet(const Read_packet& packet) {
  const auto* data = packet.payload_data();
  const auto size = packet.payload_size();
  auto error = Packet_error::none;
  switch (packet.packet_type()) {
  case Login_request_packet::packet_type:
    error = process_login_request(data, size);
    break;
  default:
    error = Packet_error::invalid_message_type;
    break;
  }
  if (error != Packet_error::none) {
    pending_reason_ = Disconnect_reason::protocol_violation;
    socket_.close();
  }
}

Packet_error Tcp_connection::process_login_request(const void*,
                                                   std::size_t size) {
  if (size != Login_request_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_ != State::connected)
    return Packet_error::unexpected_sequence;

  return Packet_error::none;
}

void Tcp_connection::terminate(Disconnect_reason reason) {
  if (state_ == State::disconnected)
    return;
  state_ = State::disconnected;
  pending_reason_ = Disconnect_reason::none;
  socket_.close();
  acceptor_->on_disconnect(reason);
}

void Tcp_connection::close() {
  pending_reason_ = Disconnect_reason::user_initiated;
  socket_.close();
}

} // namespace bc::soup::server
