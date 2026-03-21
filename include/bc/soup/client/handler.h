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

} // namespace bc::soup::client

#endif
