#include "bc/soup/server/server.h"

#include "bc/soup/validate.h"

namespace bc::soup::server {

Server::Server(asio::any_io_executor io_executor) : io_executor_(io_executor) {
}

std::error_code Server::set_session(std::string_view session) {
  if (!is_valid_session(session))
    return std::make_error_code(std::errc::invalid_argument);
  session_ = session;
  return {};
}

expected<Acceptor*, std::error_code>
Server::add_acceptor(const asio::ip::tcp::endpoint& endpoint) {
  return add_acceptor(endpoint, nullptr);
}

expected<Acceptor*, std::error_code>
Server::add_acceptor(const asio::ip::tcp::endpoint& endpoint,
                     Acceptor_handler& handler) {
  return add_acceptor(endpoint, &handler);
}

std::error_code Server::start() {
  for (const auto& acceptor : acceptors_) {
    if (!acceptor.is_handler_set())
      return std::make_error_code(std::errc::invalid_argument);
  }

  asio::post(io_executor_, [this] {
    if (started_)
      return;
    started_ = true;

    for (auto& acceptor : acceptors_)
      acceptor.start();
  });

  return {};
}

void Server::end_session() {
  asio::post(io_executor_, [this] {
    if (!started_)
      return;

    for (auto& acceptor : acceptors_)
      acceptor.end_session();
  });
}

void Server::stop() {
  asio::post(io_executor_, [this] {
    if (!started_)
      return;
    started_ = false;

    for (auto& acceptor : acceptors_)
      acceptor.stop();
  });
}

expected<Acceptor*, std::error_code>
Server::add_acceptor(const asio::ip::tcp::endpoint& endpoint,
                     Acceptor_handler* handler) {
  for (const auto& acceptor : acceptors_) {
    if (endpoint == acceptor.endpoint())
      return unexpected(std::make_error_code(std::errc::invalid_argument));
  }
  return &acceptors_.emplace_back(io_executor_, endpoint, *this, handler);
}

} // namespace bc::soup::server
