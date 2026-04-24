#include "bc/soup/types.h"

namespace bc::soup {

const char* to_string(Packet_error error) {
  switch (error) {
  case Packet_error::malformed_header:
    return "malformed header";
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
  case Disconnect_reason::user_initiated:
    return "user initiated";
  case Disconnect_reason::peer_closed:
    return "peer closed";
  case Disconnect_reason::transport_error:
    return "transport error";
  case Disconnect_reason::protocol_violation:
    return "protocol violation";
  }
  return "?";
}

} // namespace bc::soup
