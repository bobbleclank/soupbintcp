#ifndef INCLUDE_BC_SOUP_VALIDATE_H
#define INCLUDE_BC_SOUP_VALIDATE_H

#include <string_view>
#include <system_error>

namespace bc::soup {

[[nodiscard]] std::error_code validate_username(std::string_view);
[[nodiscard]] std::error_code validate_password(std::string_view);
[[nodiscard]] std::error_code validate_session(std::string_view);

} // namespace bc::soup

#endif
