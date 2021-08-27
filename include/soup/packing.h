#ifndef INCLUDE_SOUP_PACKING_H
#define INCLUDE_SOUP_PACKING_H

#include <cstring>
#include <type_traits>

#include <arpa/inet.h>

namespace soup {

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
  auto t = static_cast<T>(e);
  pack(t, data);
}

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, bool> = true>
void unpack(Enum& e, const void* data) {
  using T = std::underlying_type_t<Enum>;
  T t = 0;
  unpack(t, data);
  e = static_cast<Enum>(t);
}

} // namespace soup

#endif
