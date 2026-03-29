#include "bc/soup/server/server.h"

namespace bc::soup::server {

Server::Server(asio::any_io_executor io_executor) : io_executor_(io_executor) {
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

void Server::start() {
  asio::post(io_executor_, [this] {
    if (started_)
      return;
    started_ = true;

    for (auto& acceptor : acceptors_)
      acceptor.start();
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
