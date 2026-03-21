#ifndef INCLUDE_BC_SOUP_CLIENT_HANDLER_H
#define INCLUDE_BC_SOUP_CLIENT_HANDLER_H

namespace bc::soup::client {

class Client_handler {
public:
protected:
  Client_handler() = default;
  ~Client_handler() = default;

  Client_handler(const Client_handler&) = default;
  Client_handler& operator=(const Client_handler&) = default;

  Client_handler(Client_handler&&) = default;
  Client_handler& operator=(Client_handler&&) = default;
};

class Connection_handler {
public:
protected:
  Connection_handler() = default;
  ~Connection_handler() = default;

  Connection_handler(const Connection_handler&) = default;
  Connection_handler& operator=(const Connection_handler&) = default;

  Connection_handler(Connection_handler&&) = default;
  Connection_handler& operator=(Connection_handler&&) = default;
};

} // namespace bc::soup::client

#endif
