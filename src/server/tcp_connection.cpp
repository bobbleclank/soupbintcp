#include "bc/soup/server/tcp_connection.h"

namespace bc::soup::server {

Tcp_connection::Tcp_connection() {
}

void Tcp_connection::connect_failure(asio::error_code) {
}

void Tcp_connection::connect_success() {
}

void Tcp_connection::read_failure(asio::error_code) {
}

void Tcp_connection::read_failure(Packet_error) {
}

void Tcp_connection::read_completed(const Read_packet&) {
}

void Tcp_connection::end_of_file() {
}

void Tcp_connection::write_failure(asio::error_code) {
}

void Tcp_connection::write_completed(const Write_packet&) {
}

void Tcp_connection::write_buffer_empty() {
}

} // namespace bc::soup::server
