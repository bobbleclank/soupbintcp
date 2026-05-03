#include "bc/soup/server/tcp_connection.h"

#include "bc/soup/logical_packets.h"
#include "bc/soup/rw_packets.h"
#include "bc/soup/server/acceptor.h"
#include "bc/soup/server/handler.h"
#include "bc/soup/server/port.h"

#include <cassert>
#include <utility>

namespace bc::soup::server {

using State = Connection_state::State;

Tcp_connection::Tcp_connection(asio::any_io_executor, Socket&& socket,
                               Acceptor& acceptor)
    : acceptor_(&acceptor), socket_(std::move(socket)) {

  state_.set_state(State::connected);
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
  if (!state_.is_closing())
    process_packet(packet);
  socket_.async_read();
}

void Tcp_connection::read_aborted() {
  terminate();
}

void Tcp_connection::read_end_of_file() {
  terminate(Disconnect_reason::peer_closed);
}

void Tcp_connection::write_failure(asio::error_code) {
}

void Tcp_connection::write_success(const Write_packet&) {
  if (state_.state() == State::disconnecting)
    terminate();
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
  if (error != Packet_error::none)
    initiate_disconnect(Disconnect_reason::protocol_violation);
}

Packet_error Tcp_connection::process_login_request(const void* data,
                                                   std::size_t size) {
  if (size != Login_request_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::connected)
    return Packet_error::unexpected_sequence;

  Login_request_packet request;
  read(request, data);
  const auto result =
      acceptor_->on_login_request(*this, request, port_, handler_);
  if (!result) {
    const Login_rejected_packet& response = result.error();
    initiate_disconnect(Disconnect_reason::access_denied, true);
    Write_packet packet(response.packet_type, response.payload_size);
    write(response, packet.payload_data());
    // Discard write failure: should not fail since first packet sent
    (void)socket_.async_write(std::move(packet));
  } else {
    const Login_accepted_packet& response = *result;
    state_.set_state(State::logged_in);
    Write_packet packet(response.packet_type, response.payload_size);
    write(response, packet.payload_data());
    // Discard write failure: should not fail since first packet sent
    (void)socket_.async_write(std::move(packet));
  }

  return Packet_error::none;
}

void Tcp_connection::terminate(Disconnect_reason observed_reason) {
  const auto reason = state_.terminate(observed_reason);
  if (reason == Disconnect_reason::none)
    return;
  acceptor_->on_disconnect(reason);
  socket_.close();
}

void Tcp_connection::initiate_disconnect(Disconnect_reason reason,
                                         bool graceful) {
  const auto state_changed = state_.initiate_disconnect(reason);
  if (!state_changed || graceful)
    return;
  socket_.close();
}

void Tcp_connection::close() {
  initiate_disconnect(Disconnect_reason::user_initiated);
}

void Tcp_connection::send_end_of_session() {
  // Discard write failure: not critical
  (void)socket_.async_write(Write_packet(End_of_session_packet::packet_type));
}

} // namespace bc::soup::server
