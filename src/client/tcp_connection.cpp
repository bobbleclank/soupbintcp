#include "bc/soup/client/tcp_connection.h"

#include "bc/soup/client/connection.h"
#include "bc/soup/client/handler.h"
#include "bc/soup/constants.h"
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
      socket_(io_executor, *this),
      login_timer_(io_executor, *this, login_response_timeout),
      heartbeat_timer_(io_executor, *this, server_heartbeat_timeout) {

  handler_->connecting(connection_->endpoint());
  socket_.set_write_packets_limit(write_packets_limit);
  if (const auto ec = socket_.open()) {
    handle_connect_failure(ec, "open");
    return;
  }
  if (const auto ec = socket_.set_no_delay()) {
    handle_connect_failure(ec, "set_no_delay");
    return;
  }
  socket_.async_connect(connection_->endpoint());
}

void Tcp_connection::connect_failure(asio::error_code ec) {
  handle_connect_failure(ec, "async_connect");
}

void Tcp_connection::connect_success() {
  state_.set_state(State::connected);
  const auto local_endpoint = socket_.local_endpoint();
  const auto remote_endpoint = socket_.remote_endpoint();
  handler_->connect_success(local_endpoint, remote_endpoint);

  const Login_request_packet request = connection_->on_connect_success();
  handler_->logging_in(request);
  login_timer_stopped_ = false;
  login_timer_.start();
  socket_.async_read();
  Write_packet packet(request.packet_type, request.payload_size);
  write(request, packet.payload_data());
  // Discard write failure: should not fail since first packet sent
  (void)socket_.async_write(std::move(packet));
}

void Tcp_connection::read_failure(asio::error_code ec) {
  handle_transport_error(ec, "socket async_read");
}

void Tcp_connection::read_failure(Packet_error error) {
  handle_protocol_violation(error);
}

void Tcp_connection::read_success(const Read_packet& packet) {
  if (state_.is_closing())
    return;
  const auto error = process_packet(packet);
  if (error != Packet_error::none)
    handle_protocol_violation(error);
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

void Tcp_connection::write_failure(asio::error_code ec) {
  handle_transport_error(ec, "socket async_write");
}

void Tcp_connection::write_success(const Write_packet&) {}

void Tcp_connection::write_buffer_empty() {
  handler_->write_buffer_empty();
}

void Tcp_connection::closed() {
  socket_closed_ = true;
  maybe_signal_closed();
}

void Tcp_connection::login_timer_error(asio::error_code ec,
                                       std::string_view operation) {
  handle_transport_error(ec, operation);
}

void Tcp_connection::login_timer_expired() {
  disconnect(Disconnect_reason::login_timeout);
}

void Tcp_connection::login_timer_stopped() {
  login_timer_stopped_ = true;
  maybe_signal_closed();
}

void Tcp_connection::heartbeat_timer_error(asio::error_code ec,
                                           std::string_view operation) {
  handle_transport_error(ec, operation);
}

void Tcp_connection::heartbeat_send_due() {
  // Discard write failure: best effort
  (void)socket_.async_write(Write_packet(Client_heartbeat_packet::packet_type));
}

void Tcp_connection::heartbeat_receive_timeout() {
  disconnect(Disconnect_reason::heartbeat_timeout);
}

void Tcp_connection::heartbeat_timer_stopped() {
  heartbeat_timer_stopped_ = true;
  maybe_signal_closed();
}

Packet_error Tcp_connection::process_packet(const Read_packet& packet) {
  const auto* data = packet.payload_data();
  const auto size = packet.payload_size();
  switch (packet.packet_type()) {
  case Debug_packet::packet_type:
    process_debug(data, size);
    return Packet_error::none;
  case Login_accepted_packet::packet_type:
    return process_login_accepted(data, size);
  case Login_rejected_packet::packet_type:
    return process_login_rejected(data, size);
  case Sequenced_data_packet::packet_type:
    return process_sequenced_data(data, size);
  case Server_heartbeat_packet::packet_type:
    return process_server_heartbeat(size);
  case End_of_session_packet::packet_type:
    return process_end_of_session(size);
  default:
    return Packet_error::invalid_message_type;
  }
}

void Tcp_connection::process_debug(const void* data, std::size_t size) {
  handler_->debug(std::string_view(static_cast<const char*>(data), size));
}

Packet_error Tcp_connection::process_login_accepted(const void* data,
                                                    std::size_t size) {
  if (size != Login_accepted_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::connected)
    return Packet_error::unexpected_sequence;

  Login_accepted_packet response;
  read(response, data);
  if (response.next_sequence_number == 0)
    return Packet_error::invalid_field_value;

  login_timer_.stop();
  state_.set_state(State::logged_in);
  const auto reason = connection_->on_login_success(response);
  if (reason == Disconnect_reason::none) {
    handler_->login_success(response);
    heartbeat_timer_stopped_ = false;
    heartbeat_timer_.start();
  } else {
    disconnect(reason);
  }
  return Packet_error::none;
}

Packet_error Tcp_connection::process_login_rejected(const void* data,
                                                    std::size_t size) {
  if (size != Login_rejected_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::connected)
    return Packet_error::unexpected_sequence;

  Login_rejected_packet response;
  read(response, data);

  auto convert = [](Login_rejected_reason reason) {
    switch (reason) {
    case Login_rejected_reason::not_authorized:
      return Login_reject_reason::not_authorized;
    case Login_rejected_reason::session_not_available:
      return Login_reject_reason::session_not_available;
    }
    return Login_reject_reason::invalid_reject_reason;
  };

  login_timer_.stop();
  handler_->login_failure(convert(response.reason));
  disconnect(Disconnect_reason::access_denied);
  return Packet_error::none;
}

Packet_error Tcp_connection::process_sequenced_data(const void* data,
                                                    std::size_t size) {
  if (state_.state() != State::logged_in)
    return Packet_error::unexpected_sequence;
  if (connection_->has_session_ended())
    return Packet_error::unexpected_sequence;

  heartbeat_timer_.increment_receive_count();
  connection_->on_sequenced_data(data, size);
  return Packet_error::none;
}

Packet_error Tcp_connection::process_server_heartbeat(std::size_t size) {
  if (size != Server_heartbeat_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::logged_in)
    return Packet_error::unexpected_sequence;

  heartbeat_timer_.increment_receive_count();
  return Packet_error::none;
}

Packet_error Tcp_connection::process_end_of_session(std::size_t size) {
  if (size != End_of_session_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::logged_in)
    return Packet_error::unexpected_sequence;

  heartbeat_timer_.increment_receive_count();
  connection_->on_end_of_session();
  return Packet_error::none;
}

void Tcp_connection::handle_connect_failure(asio::error_code ec,
                                            std::string_view operation) {
  handler_->connect_failure(ec, operation);
  disconnect(Disconnect_reason::connect_failure);
}

void Tcp_connection::handle_transport_error(asio::error_code ec,
                                            std::string_view operation) {
  handler_->transport_error(ec, operation);
  disconnect(Disconnect_reason::transport_error);
}

void Tcp_connection::handle_protocol_violation(Packet_error error) {
  handler_->protocol_violation(error);
  disconnect(Disconnect_reason::protocol_violation);
}

void Tcp_connection::disconnect(Disconnect_reason reason) {
  const auto state_changed = state_.disconnect(reason);
  if (!state_changed)
    return;
  socket_.close();
  login_timer_.stop();
  heartbeat_timer_.stop();
}

void Tcp_connection::maybe_signal_closed() {
  if (!socket_closed_ || !login_timer_stopped_ || !heartbeat_timer_stopped_)
    return;
  connection_->on_closed(state_.reason());
}

Write_error Tcp_connection::send_packet(Write_packet&& packet) {
  if (state_.state() != State::logged_in)
    return Write_error::not_logged_in;
  const auto packet_type = packet.packet_type();
  const auto error = socket_.async_write(std::move(packet));
  if (packet_type == Unsequenced_data_packet::packet_type &&
      error == Write_error::none)
    heartbeat_timer_.increment_send_count();
  return error;
}

Write_error Tcp_connection::send_debug_packet(std::string_view text) {
  if (state_.state() == State::connecting || state_.is_closing())
    return Write_error::disconnected;
  return socket_.async_write(
      Write_packet(Debug_packet::packet_type, text.data(), text.size()));
}

void Tcp_connection::close() {
  disconnect(Disconnect_reason::user_initiated);
}

} // namespace bc::soup::client
