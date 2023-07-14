#ifndef INCLUDE_BC_SOUP_TYPES_H
#define INCLUDE_BC_SOUP_TYPES_H

namespace bc::soup {

enum class Packet_error {
  bad_length
};

const char* to_string(Packet_error);

enum class Write_error {
  success,
  buffer_full
};

const char* to_string(Write_error);

} // namespace bc::soup

#endif
