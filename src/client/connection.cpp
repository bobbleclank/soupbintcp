#include "bc/soup/client/connection.h"

#include "bc/soup/client/client.h"
#include "bc/soup/client/handler.h"
#include "bc/soup/logical_packets.h"
#include "bc/soup/validate.h"

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

bool Connection::is_handler_set() const {
  return handler_ != nullptr;
}

void Connection::connect() {
  connection_.emplace(io_executor_, *this, *handler_,
                      client_->write_packets_limit());
}

void Connection::disconnect() {
  if (connection_)
    connection_->close();
}

void Connection::on_connect_failure() {
}

void Connection::on_connect_success(Login_request_packet& request) {
  request.username = username_;
  request.password = password_;
  request.session = session_;
}

void Connection::on_login_success(const Login_accepted_packet& response) {
  session_ = response.session;
}

} // namespace bc::soup::client
