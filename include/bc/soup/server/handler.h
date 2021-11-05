#ifndef INCLUDE_BC_SOUP_SERVER_HANDLER_H
#define INCLUDE_BC_SOUP_SERVER_HANDLER_H

namespace bc::soup::server {

class Handler {
public:
protected:
  Handler() = default;
  ~Handler() = default;

  Handler(const Handler&) = default;
  Handler& operator=(const Handler&) = default;

  Handler(Handler&&) = default;
  Handler& operator=(Handler&&) = default;
};

} // namespace bc::soup::server

#endif
