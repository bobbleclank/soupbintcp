#include "bc/soup/server/acceptor.h"

#include "bc/soup/error.h"
#include "bc/soup/logical_packets.h"
#include "bc/soup/server/handler.h"
#include "bc/soup/server/server.h"
#include "bc/soup/socket.h"
#include "bc/soup/validate.h"

#include <algorithm>
#include <utility>

namespace bc::soup::server {

Acceptor::Acceptor(asio::any_io_executor io_executor,
                   const asio::ip::tcp::endpoint& endpoint, Server& server,
                   Acceptor_handler* handler)
    : server_(&server),
      handler_(handler),
      endpoint_(endpoint),
      acceptor_(io_executor, *this) {}

void Acceptor::accept_failure(asio::error_code ec) {
  handler_->accept_failure(ec);
  acceptor_.async_accept();
}

void Acceptor::accept_success(asio::ip::tcp::socket&& s) {
  Socket socket(std::move(s));
  socket.set_write_packets_limit(write_packets_limit_);
  const auto local_endpoint = socket.local_endpoint();
  const auto remote_endpoint = socket.remote_endpoint();
  handler_->accept_success(local_endpoint, remote_endpoint);
  auto& connection = connections_.emplace_back(acceptor_.get_executor(),
                                               std::move(socket), *this);
  if (!debug_banner_.empty())
    (void)connection.send_debug_packet(debug_banner_);
  acceptor_.async_accept();
}

void Acceptor::set_handler(Acceptor_handler& handler) {
  handler_ = &handler;
}

void Acceptor::set_write_packets_limit(std::size_t write_packets_limit) {
  write_packets_limit_ = write_packets_limit;
}

void Acceptor::set_debug_banner(std::string_view debug_banner) {
  debug_banner_ = debug_banner;
}

expected<Port*, std::error_code> Acceptor::add_port(std::string_view username,
                                                    std::string_view password) {
  return add_port(username, password, nullptr);
}

expected<Port*, std::error_code>
Acceptor::add_port(std::string_view username, std::string_view password,
                   Port_handler& port_handler) {
  return add_port(username, password, &port_handler);
}

expected<Port*, std::error_code>
Acceptor::add_port(std::string_view username, std::string_view password,
                   Port_handler* port_handler) {
  if (const auto ec = validate_username(username))
    return unexpected(ec);
  if (const auto ec = validate_password(password))
    return unexpected(ec);

  for (const auto& port : ports_) {
    if (username == port.username())
      return unexpected(Error::username_in_use);
  }
  return &ports_.emplace_back(username, password, port_handler);
}

bool Acceptor::is_handler_set() const {
  if (!handler_)
    return false;
  for (const auto& port : ports_) {
    if (!port.is_handler_set())
      return false;
  }
  return true;
}

void Acceptor::start() {
  if (const auto ec = acceptor_.open()) {
    handler_->listen_setup_failure(ec, "open");
    return;
  }
  if (const auto ec = acceptor_.set_reuse_address()) {
    handler_->listen_setup_failure(ec, "set_reuse_address");
    return;
  }
  if (const auto ec = acceptor_.set_no_delay()) {
    handler_->listen_setup_failure(ec, "set_no_delay");
    return;
  }
  if (const auto ec = acceptor_.bind(endpoint_)) {
    handler_->listen_setup_failure(ec, "bind");
    return;
  }
  if (const auto ec = acceptor_.listen()) {
    handler_->listen_setup_failure(ec, "listen");
    return;
  }
  const auto local_endpoint = acceptor_.local_endpoint();
  handler_->listen_setup_success(local_endpoint);
  acceptor_.async_accept();
}

void Acceptor::end_session() {
  for (auto& port : ports_)
    port.end_session();
}

void Acceptor::stop() {
  acceptor_.close();
  for (auto& connection : connections_)
    connection.close();
}

void Acceptor::on_debug(std::string_view text) {
  handler_->debug(text);
}

expected<Login_accepted_packet, Login_rejected_packet>
Acceptor::on_login_request(Tcp_connection& connection,
                           const Login_request_packet& request, Port*& port,
                           Port_handler*& port_handler) {
  handler_->login_request(request);
  const auto iter =
      std::find_if(ports_.begin(), ports_.end(),
                   [&username = request.username](const auto& port) {
                     return username == port.username();
                   });
  if (iter == ports_.end()) {
    handler_->login_failure(Login_reject_reason::user_not_found);
    return unexpected(
        Login_rejected_packet(Login_rejected_reason::not_authorized));
  }
  port = &*iter;
  return port->on_login_request(connection, request, server_->session(),
                                port_handler);
}

void Acceptor::on_transport_error(asio::error_code ec,
                                  std::string_view operation) {
  handler_->transport_error(ec, operation);
}

void Acceptor::on_protocol_violation(Packet_error error) {
  handler_->protocol_violation(error);
}

void Acceptor::on_closed(Tcp_connection& connection, Port_handler* port_handler,
                         Disconnect_reason reason) {
  connections_.remove_if(
      [&connection](const auto& element) { return &connection == &element; });
  if (port_handler)
    port_handler->disconnect(reason);
  else
    handler_->disconnect(reason);
}

} // namespace bc::soup::server
