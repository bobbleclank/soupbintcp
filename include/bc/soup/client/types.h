#ifndef INCLUDE_BC_SOUP_CLIENT_TYPES_H
#define INCLUDE_BC_SOUP_CLIENT_TYPES_H

namespace bc::soup::client {

enum class Login_reject_reason {
  none = 0,
  not_authorized,
  session_not_available,
  invalid_reject_reason
};

const char* to_string(Login_reject_reason);

} // namespace bc::soup::client

#endif
