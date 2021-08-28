#ifndef INCLUDE_SOUP_VALIDATE_H
#define INCLUDE_SOUP_VALIDATE_H

#include <string_view>

namespace soup {

[[nodiscard]] bool is_valid_username(std::string_view);
[[nodiscard]] bool is_valid_password(std::string_view);
[[nodiscard]] bool is_valid_session(std::string_view);

} // namespace soup

#endif
