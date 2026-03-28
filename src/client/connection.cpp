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

void Connection::connect() {
}

void Connection::disconnect() {
}

} // namespace bc::soup::client
