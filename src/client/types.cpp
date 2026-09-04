#include "bc/soup/client/types.h"

namespace bc::soup::client {

const char* to_string(Login_reject_reason reason) {
  switch (reason) {
  case Login_reject_reason::none:
    return "none";
  case Login_reject_reason::not_authorized:
    return "not authorized";
  case Login_reject_reason::session_not_available:
    return "session not available";
  case Login_reject_reason::invalid_reject_reason:
    return "invalid reject reason";
  }
  return "?";
}

} // namespace bc::soup::client
