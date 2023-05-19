#include "bc/soup/server/acceptor.h"

namespace bc::soup::server {

Acceptor::Acceptor() {}

void Acceptor::accept_failure(boost::system::error_code) {}

void Acceptor::accept_success(boost::asio::ip::tcp::socket&&) {}

} // namespace bc::soup::server
