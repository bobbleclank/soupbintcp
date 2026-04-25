#ifndef INCLUDE_BC_SOUP_TYPES_H
#define INCLUDE_BC_SOUP_TYPES_H

namespace bc::soup {

enum class Packet_error {
  none = 0,
  malformed_header,
  invalid_message_type,
  incorrect_length,
  unexpected_sequence
};

const char* to_string(Packet_error);

enum class Login_reject_reason {
  none = 0,
  user_not_found,
  incorrect_password
};

const char* to_string(Login_reject_reason);

enum class Write_error {
  none = 0,
  buffer_full
};

const char* to_string(Write_error);

enum class Disconnect_reason {
  none = 0,
  user_initiated,
  peer_closed,
  transport_error,
  protocol_violation,
  unmanaged_abort
};

const char* to_string(Disconnect_reason);

} // namespace bc::soup

#endif
