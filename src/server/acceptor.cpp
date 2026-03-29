#include "bc/soup/server/acceptor.h"

#include "bc/soup/server/handler.h"
#include "bc/soup/server/server.h"
#include "bc/soup/validate.h"

namespace bc::soup::server {

Acceptor::Acceptor(asio::any_io_executor io_executor,
                   const asio::ip::tcp::endpoint& endpoint, Server& server,
                   Acceptor_handler* handler)
    : server_(&server),
      handler_(handler),
      endpoint_(endpoint),
      acceptor_(io_executor, *this) {
}

void Acceptor::accept_failure(asio::error_code) {
}

// NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Not implemented
void Acceptor::accept_success(asio::ip::tcp::socket&&) {
}

void Acceptor::set_handler(Acceptor_handler& handler) {
  handler_ = &handler;
}

expected<Port*, std::error_code> Acceptor::add_port(std::string_view username,
                                                    std::string_view password) {
  return add_port(username, password, nullptr);
}

expected<Port*, std::error_code> Acceptor::add_port(std::string_view username,
                                                    std::string_view password,
                                                    Port_handler& handler) {
  return add_port(username, password, &handler);
}

expected<Port*, std::error_code> Acceptor::add_port(std::string_view username,
                                                    std::string_view password,
                                                    Port_handler* handler) {
  if (!is_valid_username(username))
    return unexpected(std::make_error_code(std::errc::invalid_argument));
  if (!is_valid_password(password))
    return unexpected(std::make_error_code(std::errc::invalid_argument));

  for (const auto& port : ports_) {
    if (username == port.username())
      return unexpected(std::make_error_code(std::errc::invalid_argument));
  }
  return &ports_.emplace_back(username, password, handler);
}

void Acceptor::start() {
}

void Acceptor::stop() {
}

} // namespace bc::soup::server
