#ifndef INCLUDE_BC_SOUP_SERVER_PORT_H
#define INCLUDE_BC_SOUP_SERVER_PORT_H

#include <string>
#include <string_view>

namespace bc::soup::server {

class Port_handler;

class Port {
public:
  Port(std::string_view, std::string_view, Port_handler*);

  void set_handler(Port_handler&);

  std::string_view username() const { return username_; }
  std::string_view password() const { return password_; }

private:
  Port_handler* handler_ = nullptr;
  std::string username_;
  std::string password_;

  // Called by Acceptor
  friend class Acceptor;
  bool is_handler_set() const;
};

} // namespace bc::soup::server

#endif
