#ifndef INCLUDE_BC_SOUP_SERVER_ACCEPTOR_H
#define INCLUDE_BC_SOUP_SERVER_ACCEPTOR_H

#include "bc/soup/socket_acceptor.h"

namespace bc::soup::server {

class Acceptor final : public Socket_acceptor::Handler {
public:
  Acceptor();

  void accept_failure(asio::error_code) override;
  void accept_success(asio::ip::tcp::socket&&) override;

private:
};

} // namespace bc::soup::server

#endif
