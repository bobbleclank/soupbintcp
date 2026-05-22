#include "bc/soup/client/tcp_connection.h"

#include "bc/soup/client/connection.h"
#include "bc/soup/client/handler.h"
#include "bc/soup/logical_packets.h"
#include "bc/soup/rw_packets.h"

#include <utility>

namespace bc::soup::client {

using State = Connection_state::State;

Tcp_connection::Tcp_connection(asio::any_io_executor io_executor,
                               Connection& connection,
                               Connection_handler& handler,
                               std::size_t write_packets_limit)
    : connection_(&connection),
      handler_(&handler),
      socket_(io_executor, *this) {

  handler_->connecting(connection_->endpoint());
  socket_.set_write_packets_limit(write_packets_limit);
  if (const auto ec = socket_.open()) {
    handle_connect_failure(ec, "open");
    return;
  }
  if (const auto ec = socket_.set_no_delay()) {
    handle_connect_failure(ec, "set no delay");
    return;
  }
  socket_.async_connect(connection_->endpoint());
}

void Tcp_connection::connect_failure(asio::error_code ec) {
  handle_connect_failure(ec, "connect");
}

void Tcp_connection::connect_success() {
  state_.set_state(State::connected);
  const auto local_endpoint = socket_.local_endpoint();
  const auto remote_endpoint = socket_.remote_endpoint();
  handler_->connect_success(local_endpoint, remote_endpoint);

  const Login_request_packet request = connection_->on_connect_success();
  handler_->logging_in(request);
  socket_.async_read();
  Write_packet packet(request.packet_type, request.payload_size);
  write(request, packet.payload_data());
  // Discard write failure: should not fail since first packet sent
  (void)socket_.async_write(std::move(packet));
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
  disconnect(Disconnect_reason::transport_error);
}

void Tcp_connection::write_success(const Write_packet&) {
}

void Tcp_connection::write_buffer_empty() {
  handler_->write_buffer_empty();
}

void Tcp_connection::closed() {
  connection_->on_closed(state_.reason());
}

Packet_error Tcp_connection::process_packet(const Read_packet& packet) {
  const auto* data = packet.payload_data();
  const auto size = packet.payload_size();
  switch (packet.packet_type()) {
  case Login_accepted_packet::packet_type:
    return process_login_accepted(data, size);
  case Login_rejected_packet::packet_type:
    return process_login_rejected(data, size);
  case Sequenced_data_packet::packet_type:
    return process_sequenced_data(data, size);
  case End_of_session_packet::packet_type:
    return process_end_of_session(size);
  default:
    return Packet_error::invalid_message_type;
  }
}

Packet_error Tcp_connection::process_login_accepted(const void* data,
                                                    std::size_t size) {
  if (size != Login_accepted_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::connected)
    return Packet_error::unexpected_sequence;

  Login_accepted_packet response;
  read(response, data);
  state_.set_state(State::logged_in);
  connection_->on_login_success(response);
  handler_->login_success(response);
  return Packet_error::none;
}

Packet_error Tcp_connection::process_login_rejected(const void* data,
                                                    std::size_t size) {
  if (size != Login_rejected_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::connected)
    return Packet_error::unexpected_sequence;

  auto convert = [](Login_rejected_reason reason) {
    switch (reason) {
    case Login_rejected_reason::not_authorized:
      return Login_reject_reason::not_authorized;
    case Login_rejected_reason::session_not_available:
      return Login_reject_reason::session_not_available;
    case Login_rejected_reason::sequence_number_too_high:
      break;
    }
    return Login_reject_reason::invalid_reject_reason;
  };

  Login_rejected_packet response;
  read(response, data);
  handler_->login_failure(convert(response.reason));
  disconnect(Disconnect_reason::access_denied);
  return Packet_error::none;
}

Packet_error Tcp_connection::process_sequenced_data(const void* data,
                                                    std::size_t size) {
  if (state_.state() != State::logged_in)
    return Packet_error::unexpected_sequence;

  return connection_->on_sequenced_data(data, size);
}

Packet_error Tcp_connection::process_end_of_session(std::size_t size) {
  if (size != End_of_session_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::logged_in)
    return Packet_error::unexpected_sequence;

  connection_->on_end_of_session();
  return Packet_error::none;
}

void Tcp_connection::handle_connect_failure(asio::error_code ec,
                                            const char* phase) {
  state_.set_state(State::disconnected);
  connection_->on_connect_failure();
  handler_->connect_failure(ec, phase);
  socket_.close();
}

void Tcp_connection::disconnect(Disconnect_reason reason) {
  const auto state_changed = state_.disconnect(reason);
  if (!state_changed)
    return;
  socket_.close();
}

Write_error Tcp_connection::send_packet(Write_packet&& packet) {
  if (state_.state() != State::logged_in)
    return Write_error::not_logged_in;
  return socket_.async_write(std::move(packet));
}

Write_error Tcp_connection::send_logout_request() {
  if (state_.state() != State::logged_in)
    return Write_error::not_logged_in;
  return socket_.async_write(Write_packet(Logout_request_packet::packet_type));
}

void Tcp_connection::close() {
  disconnect(Disconnect_reason::user_initiated);
}

} // namespace bc::soup::client
