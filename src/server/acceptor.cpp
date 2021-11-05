#include "bc/soup/server/acceptor.h"

namespace bc::soup::server {

Acceptor::Acceptor() {}

void Acceptor::accept_failure(asio::error_code) {}

void Acceptor::accept_success(asio::ip::tcp::socket&&) {}

} // namespace bc::soup::server
