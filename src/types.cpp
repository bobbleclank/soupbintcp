#include "soup/types.h"

namespace soup {

const char* to_string(Packet_error error) {
  switch (error) {
  case Packet_error::bad_length:
    return "bad length";
  }
  return "?";
}

const char* to_string(Write_error error) {
  switch (error) {
  case Write_error::success:
    return "success";
  case Write_error::buffer_full:
    return "buffer full";
  }
  return "?";
}

} // namespace soup
