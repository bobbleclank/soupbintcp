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
  case Packet_error::unexpected_sequence:
    return "unexpected sequence";
  }
  return "?";
}

const char* to_string(Login_reject_reason reason) {
  switch (reason) {
  case Login_reject_reason::none:
    return "none";
  case Login_reject_reason::user_not_found:
    return "user not found";
  case Login_reject_reason::incorrect_password:
    return "incorrect password";
  }
  return "?";
}

const char* to_string(Write_error error) {
  switch (error) {
  case Write_error::none:
    return "none";
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
  case Disconnect_reason::peer_closed:
    return "peer closed";
  case Disconnect_reason::transport_error:
    return "transport error";
  case Disconnect_reason::protocol_violation:
    return "protocol violation";
  case Disconnect_reason::unmanaged_abort:
    return "unmanaged abort";
  }
  return "?";
}

} // namespace bc::soup
