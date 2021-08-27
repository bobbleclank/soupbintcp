#ifndef INCLUDE_SOUP_CONSTANTS_H
#define INCLUDE_SOUP_CONSTANTS_H

#include <cstddef>
#include <cstdint>

namespace soup {

constexpr std::size_t packet_size_length = sizeof(std::uint16_t);
constexpr std::size_t packet_type_length = sizeof(char);
constexpr std::size_t packet_header_length =
    packet_size_length + packet_type_length;

constexpr std::size_t username_length = 6;
constexpr std::size_t password_length = 10;
constexpr std::size_t session_length = 10;
constexpr std::size_t sequence_number_length = 20;

} // namespace soup

#endif
