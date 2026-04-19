#include "bc/soup/server/tcp_connection.h"

#include "bc/soup/server/acceptor.h"

#include <cassert>
#include <utility>

namespace bc::soup::server {

Tcp_connection::Tcp_connection(asio::any_io_executor, Socket&& socket,
                               Acceptor& acceptor)
    : acceptor_(&acceptor), socket_(std::move(socket)) {

  socket_.set_handler(*this);
  socket_.async_read();
}

void Tcp_connection::connect_failure(asio::error_code) {
  assert(false);
}

void Tcp_connection::connect_success() {
  assert(false);
}

void Tcp_connection::read_failure(asio::error_code) {
}

void Tcp_connection::read_failure(Packet_error) {
}

void Tcp_connection::read_success(const Read_packet&) {
}

void Tcp_connection::read_aborted() {
}

void Tcp_connection::read_end_of_file() {
}

void Tcp_connection::write_failure(asio::error_code) {
}

void Tcp_connection::write_success(const Write_packet&) {
}

void Tcp_connection::write_buffer_empty() {
}

void Tcp_connection::close() {
  socket_.close();
}

} // namespace bc::soup::server
