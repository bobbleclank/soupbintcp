#include "bc/soup/client/tcp_connection.h"

namespace bc::soup::client {

Tcp_connection::Tcp_connection() {}

void Tcp_connection::connect_failure(boost::system::error_code) {}

void Tcp_connection::connect_success() {}

void Tcp_connection::read_failure(boost::system::error_code) {}

void Tcp_connection::read_failure(Packet_error) {}

void Tcp_connection::read_completed(const Read_packet&) {}

void Tcp_connection::end_of_file() {}

void Tcp_connection::write_failure(boost::system::error_code) {}

void Tcp_connection::write_completed(const Write_packet&) {}

void Tcp_connection::write_buffer_empty() {}

} // namespace bc::soup::client
