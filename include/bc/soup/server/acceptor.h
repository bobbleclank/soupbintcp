#ifndef INCLUDE_BC_SOUP_SERVER_ACCEPTOR_H
#define INCLUDE_BC_SOUP_SERVER_ACCEPTOR_H

#include "bc/soup/expected.h"
#include "bc/soup/server/port.h"
#include "bc/soup/server/tcp_connection.h"
#include "bc/soup/socket_acceptor.h"
#include "bc/soup/types.h"

#include <asio.hpp>

#include <cstddef>
#include <list>
#include <string_view>
#include <system_error>

namespace bc::soup::server {

class Server;
class Acceptor_handler;
class Port_handler;

class Acceptor final : public Socket_acceptor::Handler {
public:
  Acceptor(asio::any_io_executor, const asio::ip::tcp::endpoint&, Server&,
           Acceptor_handler*);

  void accept_failure(asio::error_code) override;
  void accept_success(asio::ip::tcp::socket&&) override;

  void set_handler(Acceptor_handler&);
  void set_write_packets_limit(std::size_t);

  [[nodiscard]] expected<Port*, std::error_code> add_port(std::string_view,
                                                          std::string_view);

  [[nodiscard]] expected<Port*, std::error_code>
  add_port(std::string_view, std::string_view, Port_handler&);

  const asio::ip::tcp::endpoint& endpoint() const { return endpoint_; }

private:
  static constexpr std::size_t default_write_packets_limit = 100;

  Server* server_ = nullptr;
  Acceptor_handler* handler_ = nullptr;
  asio::ip::tcp::endpoint endpoint_;
  Socket_acceptor acceptor_;
  std::list<Port> ports_;
  std::size_t write_packets_limit_ = default_write_packets_limit;
  std::list<Tcp_connection> connections_;

  [[nodiscard]] expected<Port*, std::error_code>
  add_port(std::string_view, std::string_view, Port_handler*);

  // Called by Server
  friend class Server;
  bool is_handler_set() const;
  void start();
  void stop();

  // Called by Tcp_connection
  friend class Tcp_connection;
  void on_disconnect(Disconnect_reason);
};

} // namespace bc::soup::server

#endif
