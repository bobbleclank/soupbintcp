#include "bc/soup/server/tcp_connection.h"

#include "bc/soup/constants.h"
#include "bc/soup/logical_packets.h"
#include "bc/soup/rw_packets.h"
#include "bc/soup/server/acceptor.h"
#include "bc/soup/server/handler.h"
#include "bc/soup/server/port.h"

#include <cassert>
#include <utility>

namespace bc::soup::server {

using State = Connection_state::State;

Tcp_connection::Tcp_connection(asio::any_io_executor io_executor,
                               Socket&& socket, Acceptor& acceptor,
                               Acceptor_handler& acceptor_handler)
    : acceptor_(&acceptor),
      acceptor_handler_(&acceptor_handler),
      socket_(std::move(socket)),
      login_timer_(io_executor, *this, login_request_timeout),
      heartbeat_timer_(io_executor, *this, client_heartbeat_timeout) {

  socket_.set_handler(*this);
  // NOLINTNEXTLINE(*-prefer-member-initializer): co-located with timer start
  login_timer_stopped_ = false;
  login_timer_.start();
  socket_.async_read();
}

void Tcp_connection::connect_failure(asio::error_code) {
  assert(false);
}

void Tcp_connection::connect_success() {
  assert(false);
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

void Tcp_connection::write_success(const Write_packet&) {
  if (state_.state() == State::disconnecting)
    disconnect();
}

void Tcp_connection::write_buffer_empty() {
  if (handler_)
    handler_->write_buffer_empty();
  // No acceptor-level write_buffer_empty; drop pre-login
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
  (void)socket_.async_write(Write_packet(Server_heartbeat_packet::packet_type));
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
  case Login_request_packet::packet_type:
    return process_login_request(data, size);
  case Unsequenced_data_packet::packet_type:
    return process_unsequenced_data(data, size);
  case Client_heartbeat_packet::packet_type:
    return process_client_heartbeat(size);
  case Logout_request_packet::packet_type:
    return process_logout_request(size);
  default:
    return Packet_error::invalid_message_type;
  }
}

void Tcp_connection::process_debug(const void* data, std::size_t size) {
  const std::string_view text(static_cast<const char*>(data), size);
  if (handler_)
    handler_->debug(text);
  else
    acceptor_handler_->debug(text);
}

Packet_error Tcp_connection::process_login_request(const void* data,
                                                   std::size_t size) {
  if (size != Login_request_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::connected)
    return Packet_error::unexpected_sequence;

  Login_request_packet request;
  read(request, data);

  login_timer_.stop();
  acceptor_handler_->login_request(request);
  const auto result =
      acceptor_->on_login_request(*this, request, port_, handler_);
  if (result) {
    const Login_accepted_packet& response = *result;
    state_.set_state(State::logged_in);
    handler_->login_success(response);
    heartbeat_timer_stopped_ = false;
    heartbeat_timer_.start();
    Write_packet packet(response.packet_type, response.payload_size);
    write(response, packet.payload_data());
    // Discard write failure: should not fail since first packet sent
    (void)socket_.async_write(std::move(packet));
  } else {
    const Login_rejected_packet& response = result.error();
    prepare_graceful_disconnect(Disconnect_reason::access_denied);
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

  heartbeat_timer_.increment_receive_count();
  if (!port_->has_session_ended())
    handler_->unsequenced_data(data, size);
  return Packet_error::none;
}

Packet_error Tcp_connection::process_client_heartbeat(std::size_t size) {
  if (size != Client_heartbeat_packet::payload_size)
    return Packet_error::incorrect_length;
  if (state_.state() != State::logged_in)
    return Packet_error::unexpected_sequence;

  heartbeat_timer_.increment_receive_count();
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

void Tcp_connection::handle_transport_error(asio::error_code ec,
                                            std::string_view operation) {
  if (handler_)
    handler_->transport_error(ec, operation);
  else
    acceptor_handler_->transport_error(ec, operation);
  disconnect(Disconnect_reason::transport_error);
}

void Tcp_connection::handle_protocol_violation(Packet_error error) {
  if (handler_)
    handler_->protocol_violation(error);
  else
    acceptor_handler_->protocol_violation(error);
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
  if (port_)
    port_->on_closed(*this);
  acceptor_->on_closed(*this, handler_, state_.reason());
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
  const auto packet_type = packet.packet_type();
  const auto error = socket_.async_write(std::move(packet));
  if (packet_type == Sequenced_data_packet::packet_type &&
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

void Tcp_connection::supersede() {
  disconnect(Disconnect_reason::superseded);
}

} // namespace bc::soup::server
