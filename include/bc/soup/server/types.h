#ifndef INCLUDE_BC_SOUP_SERVER_TYPES_H
#define INCLUDE_BC_SOUP_SERVER_TYPES_H

namespace bc::soup::server {

enum class Login_reject_reason {
  user_not_found,
  incorrect_password,
  session_ended,
  invalid_session
};

const char* to_string(Login_reject_reason);

} // namespace bc::soup::server

#endif
