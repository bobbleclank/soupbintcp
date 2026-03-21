#ifndef INCLUDE_BC_SOUP_SERVER_HANDLER_H
#define INCLUDE_BC_SOUP_SERVER_HANDLER_H

namespace bc::soup::server {

class Acceptor_handler {
public:
protected:
  Acceptor_handler() = default;
  ~Acceptor_handler() = default;

  Acceptor_handler(const Acceptor_handler&) = default;
  Acceptor_handler& operator=(const Acceptor_handler&) = default;

  Acceptor_handler(Acceptor_handler&&) = default;
  Acceptor_handler& operator=(Acceptor_handler&&) = default;
};

class Port_handler {
public:
protected:
  Port_handler() = default;
  ~Port_handler() = default;

  Port_handler(const Port_handler&) = default;
  Port_handler& operator=(const Port_handler&) = default;

  Port_handler(Port_handler&&) = default;
  Port_handler& operator=(Port_handler&&) = default;
};

} // namespace bc::soup::server

#endif
