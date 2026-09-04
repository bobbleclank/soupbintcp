#include "bc/soup/server/login_reject.h"

#include <utility>

namespace bc::soup::server {

namespace {

Login_rejected_reason convert(Login_reject_reason reason) {
  switch (reason) {
  case Login_reject_reason::user_not_found:
  case Login_reject_reason::incorrect_password:
    return Login_rejected_reason::not_authorized;
  case Login_reject_reason::session_ended:
  case Login_reject_reason::invalid_session:
    return Login_rejected_reason::session_not_available;
  }
  std::unreachable();
}

} // namespace

Login_reject::Login_reject(Login_reject_reason reason_)
    : reason(reason_), packet(convert(reason_)) {}

} // namespace bc::soup::server
