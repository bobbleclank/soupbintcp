#include "bc/soup/client/connection.h"

#include "bc/soup/client/client.h"
#include "bc/soup/client/handler.h"
#include "bc/soup/client/message.h"
#include "bc/soup/logical_packets.h"
#include "bc/soup/rw_packets.h"
#include "bc/soup/validate.h"

#include <utility>

namespace bc::soup::client {

Connection::Connection(asio::any_io_executor io_executor,
                       const asio::ip::tcp::endpoint& endpoint, Client& client,
                       Connection_handler* handler)
    : client_(&client),
      handler_(handler),
      io_executor_(io_executor),
      endpoint_(endpoint) {
}

void Connection::set_handler(Connection_handler& handler) {
  handler_ = &handler;
}

std::error_code Connection::set_username(std::string_view username) {
  if (!is_valid_username(username))
    return std::make_error_code(std::errc::invalid_argument);
  username_ = username;
  return {};
}

std::error_code Connection::set_password(std::string_view password) {
  if (!is_valid_password(password))
    return std::make_error_code(std::errc::invalid_argument);
  password_ = password;
  return {};
}

std::error_code Connection::set_session(std::string_view session) {
  if (!is_valid_session(session))
    return std::make_error_code(std::errc::invalid_argument);
  session_ = session;
  return {};
}

void Connection::connect() {
  connection_.emplace(io_executor_, *this, *handler_,
                      client_->write_packets_limit());
}

void Connection::close() {
  if (connection_)
    connection_->close();
}

Write_error Connection::send_message(const void* data, std::size_t size) {
  if (size == 0)
    return Write_error::empty_buffer;
  if (!data)
    return Write_error::null_buffer;

  Write_packet packet(Unsequenced_data_packet::packet_type, data, size);
  return send_packet(std::move(packet));
}

// NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Moved via release_packet
Write_error Connection::send_message(Message&& message) {
  if (message.payload_size() == 0)
    return Write_error::empty_buffer;
  if (!message.payload_data())
    return Write_error::null_buffer;

  auto packet = message.release_packet();
  return send_packet(std::move(packet));
}

Write_error Connection::send_packet(Write_packet&& packet) {
  if (has_session_ended_)
    return Write_error::session_ended;
  if (!connection_)
    return Write_error::disconnected;

  return connection_->send_packet(std::move(packet));
}

bool Connection::is_handler_set() const {
  return handler_ != nullptr;
}

void Connection::on_connect_failure() {
}

Login_request_packet Connection::on_connect_success() {
  return Login_request_packet(username_, password_, session_, 0);
}

void Connection::on_login_success(const Login_accepted_packet& response) {
  session_ = response.session;
}

Packet_error Connection::on_sequenced_data(const void* data, std::size_t size) {
  if (has_session_ended_)
    return Packet_error::unexpected_sequence;
  client_->on_sequenced_data(data, size);
  return Packet_error::none;
}

void Connection::on_end_of_session() {
  if (has_session_ended_)
    return;
  has_session_ended_ = true;
  client_->on_end_of_session();
}

void Connection::on_closed(Disconnect_reason reason) {
  connection_.reset();
  handler_->disconnect(reason);
}

} // namespace bc::soup::client
