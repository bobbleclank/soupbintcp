#ifndef INCLUDE_BC_SOUP_ERROR_H
#define INCLUDE_BC_SOUP_ERROR_H

#include <system_error>
#include <type_traits>

namespace bc::soup {

enum class Error {
  username_too_long = 1,
  invalid_username,
  password_too_long,
  invalid_password,
  session_too_long,
  invalid_session
};

const std::error_category& soup_category() noexcept;

inline std::error_code make_error_code(Error e) noexcept {
  return std::error_code(static_cast<int>(e), soup_category());
}

} // namespace bc::soup

namespace std {

template <>
struct is_error_code_enum<bc::soup::Error> : public true_type {};

} // namespace std

#endif
