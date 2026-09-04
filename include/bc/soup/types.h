#ifndef INCLUDE_BC_SOUP_TYPES_H
#define INCLUDE_BC_SOUP_TYPES_H

namespace bc::soup {

enum class Packet_error {
  none = 0,
  malformed_header,
  invalid_message_type,
  incorrect_length,
  invalid_field_value,
  unexpected_sequence
};

const char* to_string(Packet_error);

enum class Write_error {
  none = 0,
  empty_buffer,
  null_buffer,
  buffer_too_big,
  session_ended,
  disconnected,
  not_logged_in,
  buffer_full
};

const char* to_string(Write_error);

enum class Disconnect_reason {
  none = 0,
  user_initiated,
  superseded,
  logout_request,
  peer_closed,
  connect_failure,
  transport_error,
  login_timeout,
  heartbeat_timeout,
  access_denied,
  session_mismatch,
  sequence_number_ahead_of_session,
  sequence_number_too_low,
  sequence_number_too_high,
  protocol_violation,
  unmanaged_abort
};

const char* to_string(Disconnect_reason);

} // namespace bc::soup

#endif
