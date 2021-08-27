#ifndef INCLUDE_SOUP_CONSTANTS_H
#define INCLUDE_SOUP_CONSTANTS_H

#include <cstddef>
#include <cstdint>

namespace soup {

constexpr std::size_t packet_size_length = sizeof(std::uint16_t);
constexpr std::size_t packet_type_length = sizeof(char);
constexpr std::size_t packet_header_length =
    packet_size_length + packet_type_length;

} // namespace soup

#endif
