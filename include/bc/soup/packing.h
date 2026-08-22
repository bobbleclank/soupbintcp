#ifndef INCLUDE_BC_SOUP_PACKING_H
#define INCLUDE_BC_SOUP_PACKING_H

#include <concepts>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>

#include <arpa/inet.h>

namespace bc::soup {

template <std::integral Integral>
void pack(Integral i, void* data) {
  static_assert(sizeof(i) == 1 || sizeof(i) == 2 || sizeof(i) == 4);
  if constexpr (sizeof(i) == 2)
    i = static_cast<Integral>(htons(static_cast<std::uint16_t>(i)));
  else if constexpr (sizeof(i) == 4)
    i = static_cast<Integral>(htonl(static_cast<std::uint32_t>(i)));
  std::memcpy(data, &i, sizeof(i));
}

template <std::integral Integral>
void unpack(Integral& i, const void* data) {
  static_assert(sizeof(i) == 1 || sizeof(i) == 2 || sizeof(i) == 4);
  std::memcpy(&i, data, sizeof(i));
  if constexpr (sizeof(i) == 2)
    i = static_cast<Integral>(ntohs(static_cast<std::uint16_t>(i)));
  else if constexpr (sizeof(i) == 4)
    i = static_cast<Integral>(ntohl(static_cast<std::uint32_t>(i)));
}

namespace detail {

template <typename T>
concept Enumeration = std::is_enum_v<T>;

}

template <detail::Enumeration Enum>
void pack(Enum e, void* data) {
  using T = std::underlying_type_t<Enum>;
  const auto t = static_cast<T>(e);
  pack(t, data);
}

template <detail::Enumeration Enum>
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

void pack_session(std::string_view, void*);
void unpack_session(std::string&, const void*);

void pack_sequence_number(std::uint64_t, void*);
void unpack_sequence_number(std::uint64_t&, const void*);

} // namespace bc::soup

#endif
