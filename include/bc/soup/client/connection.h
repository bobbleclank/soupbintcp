#ifndef INCLUDE_BC_SOUP_CLIENT_CONNECTION_H
#define INCLUDE_BC_SOUP_CLIENT_CONNECTION_H

#include <asio.hpp>

namespace bc::soup::client {

class Client;
class Connection_handler;

class Connection {
public:
  Connection(asio::any_io_executor, const asio::ip::tcp::endpoint&, Client&,
             Connection_handler*);

  void set_handler(Connection_handler&);

  const asio::ip::tcp::endpoint& endpoint() const { return endpoint_; }

private:
  Client* client_ = nullptr;
  Connection_handler* handler_ = nullptr;
  asio::any_io_executor io_executor_;
  asio::ip::tcp::endpoint endpoint_;

  // Called by Client
  friend class Client;
  void connect();
  void disconnect();
};

} // namespace bc::soup::client

#endif
