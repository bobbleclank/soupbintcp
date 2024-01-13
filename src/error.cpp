#include "bc/soup/error.h"

#include <string>

namespace bc::soup {

class Error_category : public std::error_category {
public:
  const char* name() const noexcept override { return "souptcp"; }

  std::string message(int value) const override {
    switch (static_cast<Error>(value)) {
    case Error::username_too_long:
      return "username too long";
    case Error::invalid_username:
      return "invalid username";
    case Error::password_too_long:
      return "password too long";
    case Error::invalid_password:
      return "invalid password";
    case Error::session_too_long:
      return "session too long";
    case Error::invalid_session:
      return "invalid session";
    }
    return "unknown error";
  }

  std::error_condition
  default_error_condition(int value) const noexcept override {
    switch (static_cast<Error>(value)) {
    case Error::username_too_long:
    case Error::invalid_username:
    case Error::password_too_long:
    case Error::invalid_password:
    case Error::session_too_long:
    case Error::invalid_session:
      return std::errc::invalid_argument;
    }
    return std::error_condition(value, *this);
  }
};

const std::error_category& souptcp_category() noexcept {
  static Error_category instance;
  return instance;
}

} // namespace bc::soup
