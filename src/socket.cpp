#include "bc/soup/socket.h"

#include <cerrno>
#include <utility>

namespace bc::soup {

Socket::Socket(boost::asio::any_io_executor io_executor) : socket_(io_executor) {}

Socket::Socket(boost::asio::any_io_executor io_executor, Handler& handler)
    : handler_(&handler), socket_(io_executor) {}

Socket::Socket(boost::asio::ip::tcp::socket&& socket) : socket_(std::move(socket)) {}

Socket::Socket(boost::asio::ip::tcp::socket&& socket, Handler& handler)
    : handler_(&handler), socket_(std::move(socket)) {}

void Socket::set_handler(Handler& handler) { handler_ = &handler; }

void Socket::set_write_packets_limit(std::size_t write_packets_limit) {
  write_packets_limit_ = write_packets_limit;
}

boost::system::error_code Socket::open() {
  boost::system::error_code ec;
  socket_.open(boost::asio::ip::tcp::v4(), ec);
  return ec;
}

void Socket::shutdown(boost::system::error_code* error) {
  boost::system::error_code ec;
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  if (error)
    *error = ec;
}

void Socket::close(boost::system::error_code* error) {
  boost::system::error_code ec;
  socket_.close(ec);
  if (error)
    *error = ec;
}

boost::system::error_code Socket::set_no_delay() {
  boost::asio::ip::tcp::no_delay option(true);
  boost::system::error_code ec;
  socket_.set_option(option, ec);
  return ec;
}

void Socket::async_connect(const boost::asio::ip::tcp::endpoint& endpoint) {
  socket_.async_connect(endpoint, [this](boost::system::error_code ec) {
    if (ec == boost::asio::error::operation_aborted) {
      return;
    }
    if (ec) {
      handler_->connect_failure(ec);
      return;
    }
    handler_->connect_success();
  });
}

void Socket::async_read() {
  read_packet_ = Read_packet();
  read_header();
}

Write_error Socket::async_write(Write_packet&& packet) {
  auto size = write_packets_.size();
  if (size == write_packets_limit_) {
    write_buffer_was_full_ = true;
    return Write_error::buffer_full;
  }
  write_packets_.push_back(std::move(packet));
  if (size == 0)
    write_packet();
  return Write_error::success;
}

boost::asio::ip::tcp::endpoint Socket::local_endpoint(boost::system::error_code* error) const {
  boost::system::error_code ec;
  auto endpoint = socket_.local_endpoint(ec);
  if (error)
    *error = ec;
  return endpoint;
}

boost::asio::ip::tcp::endpoint Socket::remote_endpoint(boost::system::error_code* error) const {
  boost::system::error_code ec;
  auto endpoint = socket_.remote_endpoint(ec);
  if (error)
    *error = ec;
  return endpoint;
}

boost::asio::ip::tcp::socket::executor_type Socket::get_executor() {
  return socket_.get_executor();
}

void Socket::read_header() {
  auto& packet = read_packet_;
  auto buffer = boost::asio::buffer(packet.header_data(), packet.header_size());
  boost::asio::async_read(socket_, buffer, [this](boost::system::error_code ec, std::size_t n) {
    header_received(ec, n);
  });
}

void Socket::header_received(boost::system::error_code ec, std::size_t n) {
  if (ec == boost::asio::error::operation_aborted) {
    return;
  }
  if (ec == boost::asio::error::eof) {
    handler_->end_of_file();
    return;
  }
  if (ec) {
    handler_->read_failure(ec);
    return;
  }
  if (n != read_packet_.header_size()) { // Needed? Error code should be set.
    boost::system::error_code ec(ECANCELED, boost::system::system_category());
    handler_->read_failure(ec);
    return;
  }
  using Result = Read_packet::Resize_result;
  auto result = read_packet_.resize_payload();
  switch (result) {
  case Result::resized:
    read_payload();
    break;
  case Result::empty_payload: {
    Read_packet packet(std::move(read_packet_));
    handler_->read_completed(packet);
    break;
  }
  case Result::bad_packet:
    handler_->read_failure(Packet_error::bad_length);
    break;
  }
}

void Socket::read_payload() {
  auto& packet = read_packet_;
  auto buffer = boost::asio::buffer(packet.payload_data(), packet.payload_size());
  boost::asio::async_read(socket_, buffer, [this](boost::system::error_code ec, std::size_t n) {
    payload_received(ec, n);
  });
}

void Socket::payload_received(boost::system::error_code ec, std::size_t n) {
  if (ec == boost::asio::error::operation_aborted) {
    return;
  }
  if (ec == boost::asio::error::eof) {
    handler_->end_of_file();
    return;
  }
  if (ec) {
    handler_->read_failure(ec);
    return;
  }
  if (n != read_packet_.payload_size()) { // Needed? Error code should be set.
    boost::system::error_code ec(ECANCELED, boost::system::system_category());
    handler_->read_failure(ec);
    return;
  }
  Read_packet packet(std::move(read_packet_));
  handler_->read_completed(packet);
}

void Socket::write_packet() {
  auto& packet = write_packets_.front();
  auto buffer = boost::asio::buffer(packet.data(), packet.size());
  boost::asio::async_write(
      socket_, buffer,
      [this](boost::system::error_code ec, std::size_t n) { packet_sent(ec, n); });
}

void Socket::packet_sent(boost::system::error_code ec, std::size_t n) {
  if (ec == boost::asio::error::operation_aborted) {
    return;
  }
  if (ec) {
    handler_->write_failure(ec);
    return;
  }
  auto& packet = write_packets_.front();
  if (n != packet.size()) { // Needed? Error code should be set.
    boost::system::error_code ec(ECANCELED, boost::system::system_category());
    handler_->write_failure(ec);
    return;
  }
  handler_->write_completed(packet);
  write_packets_.pop_front();
  if (write_buffer_was_full_ && write_packets_.empty()) {
    write_buffer_was_full_ = false;
    handler_->write_buffer_empty();
  }
  if (!write_packets_.empty())
    write_packet();
}

} // namespace bc::soup
