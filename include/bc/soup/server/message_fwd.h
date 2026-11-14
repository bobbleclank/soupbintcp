#ifndef INCLUDE_BC_SOUP_SERVER_MESSAGE_FWD_H
#define INCLUDE_BC_SOUP_SERVER_MESSAGE_FWD_H

namespace bc::soup {

template <typename Packet>
class Basic_message;

struct Sequenced_data_packet;

} // namespace bc::soup

namespace bc::soup::server {

using Message = Basic_message<Sequenced_data_packet>;

} // namespace bc::soup::server

#endif
