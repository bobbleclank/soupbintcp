#ifndef INCLUDE_BC_SOUP_CLIENT_MESSAGE_FWD_H
#define INCLUDE_BC_SOUP_CLIENT_MESSAGE_FWD_H

namespace bc::soup {

template <typename Packet>
class Basic_message;

struct Unsequenced_data_packet;

} // namespace bc::soup

namespace bc::soup::client {

using Message = Basic_message<Unsequenced_data_packet>;

} // namespace bc::soup::client

#endif
