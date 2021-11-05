#ifndef INCLUDE_BC_SOUP_CLIENT_HANDLER_H
#define INCLUDE_BC_SOUP_CLIENT_HANDLER_H

namespace bc::soup::client {

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

} // namespace bc::soup::client

#endif
