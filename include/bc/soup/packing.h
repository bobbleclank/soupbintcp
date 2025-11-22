#ifndef INCLUDE_BC_SOUP_PACKING_H
#define INCLUDE_BC_SOUP_PACKING_H

#include "bc/soup/constants.h"

#include <cctype>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

#include <arpa/inet.h>

namespace bc::soup {

template <typename Integral,
          std::enable_if_t<std::is_integral_v<Integral>, bool> = true>
void pack(Integral i, void* data) {
  static_assert(sizeof(i) == 1 || sizeof(i) == 2 || sizeof(i) == 4);
  if constexpr (sizeof(i) == 2)
    i = htons(i);
  else if constexpr (sizeof(i) == 4)
    i = htonl(i);
  std::memcpy(data, &i, sizeof(i));
}

template <typename Integral,
          std::enable_if_t<std::is_integral_v<Integral>, bool> = true>
void unpack(Integral& i, const void* data) {
  static_assert(sizeof(i) == 1 || sizeof(i) == 2 || sizeof(i) == 4);
  std::memcpy(&i, data, sizeof(i));
  if constexpr (sizeof(i) == 2)
    i = ntohs(i);
  else if constexpr (sizeof(i) == 4)
    i = ntohl(i);
}

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, bool> = true>
void pack(Enum e, void* data) {
  using T = std::underlying_type_t<Enum>;
  const auto t = static_cast<T>(e);
  pack(t, data);
}

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, bool> = true>
void unpack(Enum& e, const void* data) {
  using T = std::underlying_type_t<Enum>;
  T t = 0;
  unpack(t, data);
  e = static_cast<Enum>(t);
}

void pack_username(std::string_view, void*);
void unpack_username(std::string&, const void*);

void pack_password(std::string_view, void*);
void unpack_password(std::string&, const void*);

// Session is alphanumeric but padded on the left with spaces.

void pack_session(std::string_view, void*);
void unpack_session(std::string&, const void*);

namespace internal {

// Numeric fields are padded on the left with spaces.

template <typename Integral, std::size_t length>
void pack_numeric(Integral i, void* data) {
  static_assert(std::numeric_limits<Integral>::digits10 + 1 == length);
  constexpr auto base = 10;
  auto* ptr = static_cast<char*>(data) + length;
  if (i == 0) {
    --ptr;
    *ptr = '0';
  }
  while (i != 0) {
    const char c = '0' + (i % base);
    i /= base;
    --ptr;
    *ptr = c;
  }
  std::memset(data, ' ', ptr - static_cast<char*>(data));
}

template <typename Integral, std::size_t length>
void unpack_numeric(Integral& i, const void* data) {
  static_assert(std::numeric_limits<Integral>::digits10 + 1 == length);
  constexpr auto base = 10;
  const auto* ptr = static_cast<const char*>(data);
  const auto* end = ptr + length;
  while (ptr != end && *ptr == ' ') {
    ++ptr;
  }
  i = 0;
  while (ptr != end) {
    if (!std::isdigit(*ptr))
      break;
    i *= base;
    i += *ptr - '0';
    ++ptr;
  }
}

} // namespace internal

inline void pack_sequence_number(std::uint64_t i, void* data) {
  internal::pack_numeric<std::uint64_t, sequence_number_length>(i, data);
}

inline void unpack_sequence_number(std::uint64_t& i, const void* data) {
  internal::unpack_numeric<std::uint64_t, sequence_number_length>(i, data);
}

} // namespace bc::soup

#endif
