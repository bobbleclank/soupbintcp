#include "bc/soup/client/client.h"

#include "bc/soup/client/handler.h"

namespace bc::soup::client {

Client::Client(asio::any_io_executor io_executor) : io_executor_(io_executor) {
}

Client::Client(asio::any_io_executor io_executor, Client_handler& handler)
    : handler_(&handler), io_executor_(io_executor) {
}

void Client::set_handler(Client_handler& handler) {
  handler_ = &handler;
}

expected<Connection*, std::error_code>
Client::add_connection(const asio::ip::tcp::endpoint& endpoint) {
  return add_connection(endpoint, nullptr);
}

expected<Connection*, std::error_code>
Client::add_connection(const asio::ip::tcp::endpoint& endpoint,
                       Connection_handler& handler) {
  return add_connection(endpoint, &handler);
}

std::error_code Client::start() {
  if (!handler_)
    return std::make_error_code(std::errc::invalid_argument);
  for (const auto& connection : connections_) {
    if (!connection.is_handler_set())
      return std::make_error_code(std::errc::invalid_argument);
  }

  asio::post(io_executor_, [this] {
    if (started_)
      return;
    started_ = true;

    for (auto& connection : connections_)
      connection.connect();
  });

  return {};
}

void Client::stop() {
  asio::post(io_executor_, [this] {
    if (!started_)
      return;
    started_ = false;

    for (auto& connection : connections_)
      connection.disconnect();
  });
}

expected<Connection*, std::error_code>
Client::add_connection(const asio::ip::tcp::endpoint& endpoint,
                       Connection_handler* handler) {
  for (const auto& connection : connections_) {
    if (endpoint == connection.endpoint())
      return unexpected(std::make_error_code(std::errc::invalid_argument));
  }
  return &connections_.emplace_back(io_executor_, endpoint, *this, handler);
}

} // namespace bc::soup::client
