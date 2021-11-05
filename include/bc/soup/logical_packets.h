#ifndef INCLUDE_BC_SOUP_LOGICAL_PACKETS_H
#define INCLUDE_BC_SOUP_LOGICAL_PACKETS_H

#include "bc/soup/constants.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace bc::soup {

struct Debug_packet {
  static constexpr char packet_type = '+';
  static constexpr std::size_t payload_size = 0;
};

// Logical packets sent by the server.

struct Login_accepted_packet {
  static constexpr char packet_type = 'A';
  static constexpr std::size_t payload_size =
      session_length + sequence_number_length;

  Login_accepted_packet() = default;
  Login_accepted_packet(std::string_view, std::uint64_t);

  std::string session;
  std::uint64_t next_sequence_number = 1;
};

void read(Login_accepted_packet&, const void*);
void write(const Login_accepted_packet&, void*);

struct Login_rejected_packet {
  enum class Reason : char {
    not_authorized = 'A',
    session_not_available = 'S',
    sequence_number_too_high = 'N'
  };

  static constexpr char packet_type = 'J';
  static constexpr std::size_t payload_size = sizeof(Reason);

  Login_rejected_packet() = default;
  explicit Login_rejected_packet(Reason);

  Reason reason = Reason::not_authorized;
};

void read(Login_rejected_packet&, const void*);
void write(const Login_rejected_packet&, void*);

struct Sequenced_data_packet {
  static constexpr char packet_type = 'S';
};

struct Server_heartbeat_packet {
  static constexpr char packet_type = 'H';
  static constexpr std::size_t payload_size = 0;
};

struct End_of_session_packet {
  static constexpr char packet_type = 'Z';
  static constexpr std::size_t payload_size = 0;
};

// Logical packets sent by the client.

struct Login_request_packet {
  static constexpr char packet_type = 'L';
  static constexpr std::size_t payload_size = username_length +
                                              password_length + session_length +
                                              sequence_number_length;

  Login_request_packet() = default;
  Login_request_packet(std::string_view, std::string_view, std::string_view,
                       std::uint64_t);

  std::string username;
  std::string password;
  std::string requested_session;
  std::uint64_t requested_sequence_number = 0;
};

void read(Login_request_packet&, const void*);
void write(const Login_request_packet&, void*);

struct Unsequenced_data_packet {
  static constexpr char packet_type = 'U';
};

struct Client_heartbeat_packet {
  static constexpr char packet_type = 'R';
  static constexpr std::size_t payload_size = 0;
};

struct Logout_request_packet {
  static constexpr char packet_type = 'O';
  static constexpr std::size_t payload_size = 0;
};

} // namespace bc::soup

#endif
