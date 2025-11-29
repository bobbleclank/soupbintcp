#include "bc/soup/packing.h"

#include <algorithm>
#include <span>

namespace bc::soup {
namespace {

// Username and password are alphanumeric and padded on the right with spaces.

template <std::size_t length>
void pack_alphanumeric_right_padded(std::string_view str, void* data) {
  auto* ptr = static_cast<char*>(data);
  if (str.size() > length)
    str.remove_suffix(str.size() - length);
  const auto iter =
      std::ranges::find_if_not(str, [](auto c) { return std::isalnum(c); });
  const std::size_t i = iter - str.begin();
  std::memcpy(ptr, str.data(), i);
  std::memset(ptr + i, ' ', length - i);
}

template <std::size_t length>
void unpack_alphanumeric_right_padded(std::string& str, const void* data) {
  const std::span<const char, length> s(static_cast<const char*>(data), length);
  const auto iter =
      std::ranges::find_if_not(s, [](auto c) { return std::isalnum(c); });
  const auto sub = s.first(iter - s.begin());
  str.insert(str.begin(), sub.begin(), sub.end());
}

} // namespace

void pack_username(std::string_view str, void* data) {
  pack_alphanumeric_right_padded<username_length>(str, data);
}

void unpack_username(std::string& str, const void* data) {
  unpack_alphanumeric_right_padded<username_length>(str, data);
}

void pack_password(std::string_view str, void* data) {
  pack_alphanumeric_right_padded<password_length>(str, data);
}

void unpack_password(std::string& str, const void* data) {
  unpack_alphanumeric_right_padded<password_length>(str, data);
}

namespace {

// Session is alphanumeric and padded on the left with spaces.

template <std::size_t length>
void pack_alphanumeric_left_padded(std::string_view str, void* data) {
  auto* ptr = static_cast<char*>(data);
  if (str.size() > length)
    str.remove_suffix(str.size() - length);
  const auto iter = std::ranges::find_if_not(
      str.rbegin(), str.rend(), [](auto c) { return std::isalnum(c); });
  const std::size_t i = iter - str.rbegin();
  std::memset(ptr, ' ', length - i);
  std::memcpy(ptr + (length - i), str.end() - i, i);
}

template <std::size_t length>
void unpack_alphanumeric_left_padded(std::string& str, const void* data) {
  const std::span<const char, length> s(static_cast<const char*>(data), length);
  const auto iter = std::ranges::find_if_not(
      s.rbegin(), s.rend(), [](auto c) { return std::isalnum(c); });
  const auto sub = s.last(iter - s.rbegin());
  str.insert(str.begin(), sub.begin(), sub.end());
}

} // namespace

void pack_session(std::string_view str, void* data) {
  pack_alphanumeric_left_padded<session_length>(str, data);
}

void unpack_session(std::string& str, const void* data) {
  unpack_alphanumeric_left_padded<session_length>(str, data);
}

} // namespace bc::soup
