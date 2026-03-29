#ifndef INCLUDE_BC_SOUP_SERVER_ACCEPTOR_H
#define INCLUDE_BC_SOUP_SERVER_ACCEPTOR_H

#include "bc/soup/socket_acceptor.h"

#include <asio.hpp>

namespace bc::soup::server {

class Server;
class Acceptor_handler;

class Acceptor final : public Socket_acceptor::Handler {
public:
  Acceptor(asio::any_io_executor, const asio::ip::tcp::endpoint&, Server&,
           Acceptor_handler*);

  void accept_failure(asio::error_code) override;
  void accept_success(asio::ip::tcp::socket&&) override;

  void set_handler(Acceptor_handler&);

  const asio::ip::tcp::endpoint& endpoint() const { return endpoint_; }

private:
  Server* server_ = nullptr;
  Acceptor_handler* handler_ = nullptr;
  asio::ip::tcp::endpoint endpoint_;
  Socket_acceptor acceptor_;

  // Called by Server
  friend class Server;
  void start();
  void stop();
};

} // namespace bc::soup::server

#endif
