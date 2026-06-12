#include "bc/soup/types.h"

namespace bc::soup {

const char* to_string(Packet_error error) {
  switch (error) {
  case Packet_error::none:
    return "none";
  case Packet_error::malformed_header:
    return "malformed header";
  case Packet_error::invalid_message_type:
    return "invalid message type";
  case Packet_error::incorrect_length:
    return "incorrect length";
  case Packet_error::invalid_field_value:
    return "invalid field value";
  case Packet_error::unexpected_sequence:
    return "unexpected sequence";
  }
  return "?";
}

const char* to_string(Login_reject_reason reason) {
  switch (reason) {
  case Login_reject_reason::none:
    return "none";
  case Login_reject_reason::not_authorized:
    return "not authorized";
  case Login_reject_reason::user_not_found:
    return "user not found";
  case Login_reject_reason::incorrect_password:
    return "incorrect password";
  case Login_reject_reason::session_not_available:
    return "session not available";
  case Login_reject_reason::invalid_reject_reason:
    return "invalid reject reason";
  }
  return "?";
}

const char* to_string(Write_error error) {
  switch (error) {
  case Write_error::none:
    return "none";
  case Write_error::empty_buffer:
    return "empty buffer";
  case Write_error::null_buffer:
    return "null buffer";
  case Write_error::session_ended:
    return "session ended";
  case Write_error::disconnected:
    return "disconnected";
  case Write_error::not_logged_in:
    return "not logged in";
  case Write_error::buffer_full:
    return "buffer full";
  }
  return "?";
}

const char* to_string(Disconnect_reason reason) {
  switch (reason) {
  case Disconnect_reason::none:
    return "none";
  case Disconnect_reason::user_initiated:
    return "user initiated";
  case Disconnect_reason::logout_request:
    return "logout request";
  case Disconnect_reason::peer_closed:
    return "peer closed";
  case Disconnect_reason::connect_failure:
    return "connect failure";
  case Disconnect_reason::transport_error:
    return "transport error";
  case Disconnect_reason::login_timeout:
    return "login timeout";
  case Disconnect_reason::heartbeat_timeout:
    return "heartbeat timeout";
  case Disconnect_reason::access_denied:
    return "access denied";
  case Disconnect_reason::session_mismatch:
    return "session mismatch";
  case Disconnect_reason::sequence_number_ahead_of_session:
    return "sequence number ahead of session";
  case Disconnect_reason::sequence_number_too_low:
    return "sequence number too low";
  case Disconnect_reason::sequence_number_too_high:
    return "sequence number too high";
  case Disconnect_reason::protocol_violation:
    return "protocol violation";
  case Disconnect_reason::unmanaged_abort:
    return "unmanaged abort";
  }
  return "?";
}

} // namespace bc::soup
