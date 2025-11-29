#include "bc/soup/packing.h"

namespace bc::soup {
namespace {

// Username and password are alphanumeric and padded on the right with spaces.

template <std::size_t length>
void pack_alphanumeric_right_padded(std::string_view str, void* data) {
  auto* ptr = static_cast<char*>(data);
  if (str.size() > length)
    str.remove_suffix(str.size() - length);
  std::size_t i = 0;
  while (i != str.size() && std::isalnum(str[i])) {
    ++i;
  }
  std::memcpy(ptr, str.data(), i);
  std::memset(ptr + i, ' ', length - i);
}

template <std::size_t length>
void unpack_alphanumeric_right_padded(std::string& str, const void* data) {
  std::string_view sv(static_cast<const char*>(data), length);
  std::size_t i = 0;
  while (i != sv.size() && std::isalnum(sv[i])) {
    ++i;
  }
  sv.remove_suffix(sv.size() - i);
  str.insert(str.begin(), sv.begin(), sv.end());
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
  std::size_t i = str.size();
  while (i != 0 && std::isalnum(str[i - 1])) {
    --i;
  }
  i = str.size() - i;
  std::memset(ptr, ' ', length - i);
  std::memcpy(ptr + (length - i), str.end() - i, i);
}

template <std::size_t length>
void unpack_alphanumeric_left_padded(std::string& str, const void* data) {
  std::string_view sv(static_cast<const char*>(data), length);
  std::size_t i = sv.size();
  while (i != 0 && std::isalnum(sv[i - 1])) {
    --i;
  }
  sv.remove_prefix(i);
  str.insert(str.begin(), sv.begin(), sv.end());
}

} // namespace

void pack_session(std::string_view str, void* data) {
  pack_alphanumeric_left_padded<session_length>(str, data);
}

void unpack_session(std::string& str, const void* data) {
  unpack_alphanumeric_left_padded<session_length>(str, data);
}

} // namespace bc::soup
