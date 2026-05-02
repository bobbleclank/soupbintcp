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
  state_.set_state(State::connected);
  const auto local_endpoint = socket_.local_endpoint();
  const auto remote_endpoint = socket_.remote_endpoint();
  handler_->connect_success(local_endpoint, remote_endpoint);

  Login_request_packet request;
  connection_->on_connect_success(request);
  handler_->logging_in(request);
  socket_.async_read();
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
}

void Tcp_connection::write_buffer_empty() {
}

void Tcp_connection::process_packet(const Read_packet& packet) {
  const auto* data = packet.payload_data();
  const auto size = packet.payload_size();
  auto error = Packet_error::none;
  switch (packet.packet_type()) {
  case Login_accepted_packet::packet_type:
    error = process_login_accepted(data, size);
    break;
  case Login_rejected_packet::packet_type:
    error = process_login_rejected(data, size);
    break;
  default:
    error = Packet_error::invalid_message_type;
    break;
  }
  if (error != Packet_error::none)
    initiate_disconnect(Disconnect_reason::protocol_violation);
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
      break;
    case Login_rejected_reason::sequence_number_too_high:
      break;
    }
    return Login_reject_reason::invalid_reject_reason;
  };

  Login_rejected_packet response;
  read(response, data);
  handler_->login_failure(convert(response.reason));
  initiate_disconnect(Disconnect_reason::access_denied);
  return Packet_error::none;
}

void Tcp_connection::handle_connect_failure(asio::error_code ec,
                                            const char* phase) {
  state_.set_state(State::disconnected);
  handler_->connect_failure(ec, phase);
  connection_->on_connect_failure();
  socket_.close();
}

void Tcp_connection::terminate(Disconnect_reason observed_reason) {
  const auto reason = state_.terminate(observed_reason);
  if (reason == Disconnect_reason::none)
    return;
  handler_->disconnect(reason);
  socket_.close();
}

void Tcp_connection::initiate_disconnect(Disconnect_reason reason) {
  const auto state_changed = state_.initiate_disconnect(reason);
  if (!state_changed)
    return;
  socket_.close();
}

void Tcp_connection::close() {
  initiate_disconnect(Disconnect_reason::user_initiated);
}

} // namespace bc::soup::client
