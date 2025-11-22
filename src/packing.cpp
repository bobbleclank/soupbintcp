#include "bc/soup/packing.h"

namespace bc::soup {
namespace {

// Username and password are alphanumeric and padded on the right with spaces.

template <std::size_t length>
void pack_alphanumeric_right_padded(std::string_view str, void* data) {
  auto* ptr = static_cast<char*>(data);
  const std::size_t size = str.size() > length ? length : str.size();
  std::size_t i = 0;
  while (i != size && std::isalnum(str[i])) {
    ptr[i] = str[i];
    ++i;
  }
  std::memset(ptr + i, ' ', length - i);
}

template <std::size_t length>
void unpack_alphanumeric_right_padded(std::string& str, const void* data) {
  const auto* ptr = static_cast<const char*>(data);
  str.reserve(length);
  std::size_t i = 0;
  while (i != length && std::isalnum(ptr[i])) {
    str.push_back(ptr[i]);
    ++i;
  }
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
  const std::size_t size = str.size() > length ? length : str.size();
  std::size_t i = 0;
  while (i != size && std::isalnum(str[i])) {
    ++i;
  }
  std::memset(ptr, ' ', length - i);
  std::memcpy(ptr + (length - i), str.data(), i);
}

template <std::size_t length>
void unpack_alphanumeric_left_padded(std::string& str, const void* data) {
  const auto* ptr = static_cast<const char*>(data);
  std::size_t i = 0;
  while (i != length && ptr[i] == ' ') {
    ++i;
  }
  str.reserve(length - i);
  while (i != length && std::isalnum(ptr[i])) {
    str.push_back(ptr[i]);
    ++i;
  }
}

} // namespace

void pack_session(std::string_view str, void* data) {
  pack_alphanumeric_left_padded<session_length>(str, data);
}

void unpack_session(std::string& str, const void* data) {
  unpack_alphanumeric_left_padded<session_length>(str, data);
}

} // namespace bc::soup
