#include "bc/soup/server/types.h"

namespace bc::soup::server {

const char* to_string(Login_reject_reason reason) {
  switch (reason) {
  case Login_reject_reason::none:
    return "none";
  case Login_reject_reason::user_not_found:
    return "user not found";
  case Login_reject_reason::incorrect_password:
    return "incorrect password";
  case Login_reject_reason::session_ended:
    return "session ended";
  case Login_reject_reason::invalid_session:
    return "invalid session";
  }
  return "?";
}

} // namespace bc::soup::server
