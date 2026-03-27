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

} // namespace bc::soup
