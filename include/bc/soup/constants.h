#ifndef INCLUDE_BC_SOUP_CONSTANTS_H
#define INCLUDE_BC_SOUP_CONSTANTS_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace bc::soup {

inline constexpr std::size_t packet_size_length = sizeof(std::uint16_t);
inline constexpr std::size_t packet_type_length = sizeof(char);
inline constexpr std::size_t packet_header_length =
    packet_size_length + packet_type_length;

inline constexpr std::size_t max_payload_size =
    std::numeric_limits<std::uint16_t>::max() - packet_type_length;

inline constexpr std::size_t username_length = 6;
inline constexpr std::size_t password_length = 10;
inline constexpr std::size_t session_length = 10;
inline constexpr std::size_t sequence_number_length = 20;

inline constexpr std::chrono::seconds heartbeat_period(1);
inline constexpr std::chrono::seconds client_heartbeat_timeout(15);
inline constexpr std::chrono::seconds server_heartbeat_timeout(5);
inline constexpr std::chrono::seconds login_request_timeout(30);
inline constexpr std::chrono::seconds login_response_timeout(10);

inline constexpr std::chrono::seconds reconnect_initial_delay(1);
inline constexpr std::chrono::seconds reconnect_max_delay(30);
inline constexpr int reconnect_delay_multiplier = 2;

} // namespace bc::soup

#endif
