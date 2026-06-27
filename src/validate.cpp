#include "bc/soup/validate.h"

#include "bc/soup/constants.h"
#include "bc/soup/error.h"

#include <cctype>

namespace bc::soup {
namespace {

bool is_valid_string(std::string_view str) {
  for (const auto c : str) {
    if (!std::isalnum(c))
      return false;
  }
  return true;
}

} // namespace

std::error_code validate_username(std::string_view username) {
  if (username.size() > username_length)
    return Error::username_too_long;
  if (!is_valid_string(username))
    return Error::invalid_username;
  return {};
}

std::error_code validate_password(std::string_view password) {
  if (password.size() > password_length)
    return Error::password_too_long;
  if (!is_valid_string(password))
    return Error::invalid_password;
  return {};
}

std::error_code validate_session(std::string_view session) {
  if (session.size() > session_length)
    return Error::session_too_long;
  if (!is_valid_string(session))
    return Error::invalid_session;
  return {};
}

} // namespace bc::soup
