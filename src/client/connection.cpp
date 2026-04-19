#include "bc/soup/client/connection.h"

#include "bc/soup/client/client.h"
#include "bc/soup/client/handler.h"

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

} // namespace bc::soup::client
