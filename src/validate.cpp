#include "bc/soup/validate.h"

#include "bc/soup/constants.h"

#include <cctype>

namespace bc::soup {
namespace {

bool is_valid_string(std::string_view str) {
  for (auto c : str) {
    if (!std::isalnum(c))
      return false;
  }
  return true;
}

} // namespace

bool is_valid_username(std::string_view username) {
  if (username.size() > username_length)
    return false;
  return is_valid_string(username);
}

bool is_valid_password(std::string_view password) {
  if (password.size() > password_length)
    return false;
  return is_valid_string(password);
}

bool is_valid_session(std::string_view session) {
  if (session.size() > session_length)
    return false;
  return is_valid_string(session);
}

} // namespace bc::soup
