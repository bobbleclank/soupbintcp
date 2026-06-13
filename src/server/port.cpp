#include "bc/soup/server/port.h"

#include "bc/soup/logical_packets.h"
#include "bc/soup/rw_packets.h"
#include "bc/soup/server/handler.h"
#include "bc/soup/server/message.h"
#include "bc/soup/server/tcp_connection.h"

#include <utility>

namespace bc::soup::server {

Port::Port(std::string_view username, std::string_view password,
           Port_handler* handler)
    : handler_(handler), username_(username), password_(password) {}

void Port::set_handler(Port_handler& handler) {
  handler_ = &handler;
}

void Port::set_next_sequence_number(std::uint64_t next_sequence_number) {
  next_sequence_number_ = next_sequence_number;
}

Write_error Port::send_message(const void* data, std::size_t size) {
  if (size == 0)
    return Write_error::empty_buffer;
  if (!data)
    return Write_error::null_buffer;

  Write_packet packet(Sequenced_data_packet::packet_type, data, size);
  const auto error = send_packet(std::move(packet));
  if (error == Write_error::none)
    ++next_sequence_number_;
  return error;
}

// NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Moved via release_packet
Write_error Port::send_message(Message&& message) {
  if (message.payload_size() == 0)
    return Write_error::empty_buffer;
  if (!message.payload_data())
    return Write_error::null_buffer;

  auto packet = message.release_packet();
  const auto error = send_packet(std::move(packet));
  if (error == Write_error::none)
    ++next_sequence_number_;
  return error;
}

Write_error Port::send_debug(std::string_view text) {
  if (text.empty())
    return Write_error::empty_buffer;
  if (!connection_)
    return Write_error::disconnected;

  return connection_->send_debug_packet(text);
}

Write_error Port::send_packet(Write_packet&& packet) {
  if (has_session_ended_)
    return Write_error::session_ended;
  if (!connection_)
    return Write_error::disconnected;

  return connection_->send_packet(std::move(packet));
}

bool Port::is_handler_set() const {
  return handler_ != nullptr;
}

expected<Login_accepted_packet, Login_rejected_packet>
Port::on_login_request(Tcp_connection& connection,
                       const Login_request_packet& request,
                       std::string_view session, Port_handler*& handler) {
  handler = handler_;
  if (connection_)
    connection_->supersede();
  connection_ = &connection;

  if (request.password != password_) {
    handler_->login_failure(Login_reject_reason::incorrect_password);
    return unexpected(
        Login_rejected_packet(Login_rejected_reason::not_authorized));
  }
  if (!request.session.empty() && request.session != session) {
    handler_->login_failure(Login_reject_reason::session_not_available);
    return unexpected(
        Login_rejected_packet(Login_rejected_reason::session_not_available));
  }

  if (request.next_sequence_number != 0 &&
      request.next_sequence_number < next_sequence_number_) {
    next_sequence_number_ = request.next_sequence_number;
  }
  const Login_accepted_packet response(session, next_sequence_number_);
  handler_->login_success(response);
  return response;
}

void Port::end_session() {
  if (has_session_ended_)
    return;
  has_session_ended_ = true;
  if (connection_)
    // Discard write failure: best effort
    (void)connection_->send_packet(
        Write_packet(End_of_session_packet::packet_type));
}

void Port::on_unsequenced_data(const void* data, std::size_t size) {
  if (has_session_ended_)
    return;
  handler_->unsequenced_data(data, size);
}

void Port::on_closed(Tcp_connection& connection) {
  if (connection_ == &connection)
    connection_ = nullptr;
}

} // namespace bc::soup::server
