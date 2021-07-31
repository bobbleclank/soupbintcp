#ifndef INCLUDE_SOUP_TYPES_H
#define INCLUDE_SOUP_TYPES_H

namespace soup {

enum class Packet_error { bad_length };

const char* to_string(Packet_error);

enum class Write_error { success, buffer_full };

const char* to_string(Write_error);

} // namespace soup

#endif
