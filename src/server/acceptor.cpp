#include "bc/soup/server/acceptor.h"

namespace bc::soup::server {

Acceptor::Acceptor() {
}

void Acceptor::accept_failure(asio::error_code) {
}

// NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Not implemented
void Acceptor::accept_success(asio::ip::tcp::socket&&) {
}

} // namespace bc::soup::server
