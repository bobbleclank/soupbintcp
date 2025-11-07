#include "bc/soup/packing.h"

namespace bc::soup {
namespace internal {

void pack_alphanumeric(std::string_view str, void* data, std::size_t length) {
  auto* ptr = static_cast<char*>(data);
  const std::size_t size = str.size() > length ? length : str.size();
  std::size_t i = 0;
  while (i != size && std::isalnum(str[i])) {
    ptr[i] = str[i];
    ++i;
  }
  std::memset(ptr + i, ' ', length - i);
}

void unpack_alphanumeric(std::string& str, const void* data,
                         std::size_t length) {
  const auto* ptr = static_cast<const char*>(data);
  str.reserve(length);
  std::size_t i = 0;
  while (i != length && std::isalnum(ptr[i])) {
    str.push_back(ptr[i]);
    ++i;
  }
}

} // namespace internal

void pack_session(std::string_view str, void* data) {
  auto* ptr = static_cast<char*>(data);
  constexpr std::size_t length = session_length;
  const std::size_t size = str.size() > length ? length : str.size();
  std::size_t i = 0;
  while (i != size && std::isalnum(str[i])) {
    ++i;
  }
  std::memset(ptr, ' ', length - i);
  std::memcpy(ptr + (length - i), str.data(), i);
}

void unpack_session(std::string& str, const void* data) {
  const auto* ptr = static_cast<const char*>(data);
  constexpr std::size_t length = session_length;
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

} // namespace bc::soup
