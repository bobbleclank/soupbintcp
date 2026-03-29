#include "bc/soup/server/acceptor.h"

#include "bc/soup/server/handler.h"
#include "bc/soup/server/server.h"

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

void Acceptor::start() {
}

void Acceptor::stop() {
}

} // namespace bc::soup::server
