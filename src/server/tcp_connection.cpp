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
  disconnect(Disconnect_reason::transport_error);
}

void Tcp_connection::read_failure(Packet_error) {
  disconnect(Disconnect_reason::protocol_violation);
}

void Tcp_connection::read_success(const Read_packet& packet) {
  if (state_.is_closing())
    return;
  const auto error = process_packet(packet);
  if (error != Packet_error::none)
    disconnect(Disconnect_reason::protocol_violation);
  if (state_.is_closing())
    return;
  socket_.async_read();
}

void Tcp_connection::read_aborted() {
  disconnect();
}

void Tcp_connection::read_end_of_file() {
  disconnect(Disconnect_reason::peer_closed);
}

void Tcp_connection::write_failure(asio::error_code) {
}

void Tcp_connection::write_success(const Write_packet&) {
  if (state_.state() == State::disconnecting)
    disconnect();
}

void Tcp_connection::write_buffer_empty() {
}

void Tcp_connection::closed() {
  if (port_)
    port_->on_closed();
  acceptor_->on_closed(*this, handler_, state_.reason());
}

Packet_error Tcp_connection::process_packet(const Read_packet& packet) {
  const auto* data = packet.payload_data();
  const auto size = packet.payload_size();
  switch (packet.packet_type()) {
  case Login_request_packet::packet_type:
    return process_login_request(data, size);
  case Unsequenced_data_packet::packet_type:
    return process_unsequenced_data(data, size);
  case Logout_request_packet::packet_type:
    return process_logout_request(size);
  default:
    return Packet_error::invalid_message_type;
  }
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
    prepare_graceful_disconnect(Disconnect_reason::access_denied);
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

Packet_error Tcp_connection::process_unsequenced_data(const void* data,
                                                      std::size_t size) {
  if (state_.state() != State::logged_in)
    return Packet_error::unexpected_sequence;

  port_->on_unsequenced_data(data, size);
  return Packet_error::none;
}

Packet_error Tcp_connection::process_logout_request(std::size_t size) {
  if (size != Logout_request_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::logged_in)
    return Packet_error::unexpected_sequence;

  handler_->logout_request();
  disconnect(Disconnect_reason::logout_request);
  return Packet_error::none;
}

void Tcp_connection::disconnect(Disconnect_reason reason) {
  const auto state_changed = state_.disconnect(reason);
  if (!state_changed)
    return;
  socket_.close();
}

void Tcp_connection::prepare_graceful_disconnect(Disconnect_reason reason) {
  state_.initiate_disconnect(reason);
}

void Tcp_connection::close() {
  disconnect(Disconnect_reason::user_initiated);
}

Write_error Tcp_connection::send_packet(Write_packet&& packet) {
  if (state_.state() != State::logged_in)
    return Write_error::not_logged_in;
  return socket_.async_write(std::move(packet));
}

Write_error Tcp_connection::send_end_of_session() {
  if (state_.state() != State::logged_in)
    return Write_error::not_logged_in;
  return socket_.async_write(Write_packet(End_of_session_packet::packet_type));
}

} // namespace bc::soup::server
