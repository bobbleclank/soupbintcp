#include "bc/soup/client/client.h"

#include "bc/soup/client/handler.h"
#include "bc/soup/client/message.h"
#include "bc/soup/error.h"
#include "bc/soup/logical_packets.h"
#include "bc/soup/rw_packets.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

namespace bc::soup::client {

Client::Client(asio::any_io_executor io_executor) : io_executor_(io_executor) {}

Client::Client(asio::any_io_executor io_executor, Client_handler& handler)
    : handler_(&handler), io_executor_(io_executor) {}

void Client::set_handler(Client_handler& handler) {
  handler_ = &handler;
}

void Client::set_write_packets_limit(std::size_t write_packets_limit) {
  write_packets_limit_ = write_packets_limit;
}

void Client::set_next_sequence_number(std::uint64_t next_sequence_number) {
  next_sequence_number_ = next_sequence_number;
}

expected<Connection*, std::error_code>
Client::add_connection(const asio::ip::tcp::endpoint& endpoint) {
  return add_connection(endpoint, nullptr);
}

expected<Connection*, std::error_code>
Client::add_connection(const asio::ip::tcp::endpoint& endpoint,
                       Connection_handler& handler) {
  return add_connection(endpoint, &handler);
}

std::error_code Client::start() {
  if (!handler_)
    return Error::handler_not_set;
  for (const auto& connection : connections_) {
    if (!connection.is_handler_set())
      return Error::handler_not_set;
  }

  asio::post(io_executor_, [this] {
    if (started_)
      return;
    started_ = true;

    for (auto& connection : connections_)
      connection.connect();
  });

  return {};
}

void Client::stop() {
  asio::post(io_executor_, [this] {
    if (!started_)
      return;
    started_ = false;

    for (auto& connection : connections_)
      connection.close();
  });
}

Write_error Client::send_message(const void* data, std::size_t size) {
  if (size == 0)
    return Write_error::empty_buffer;
  if (!data)
    return Write_error::null_buffer;

  Write_packet packet(Unsequenced_data_packet::packet_type, data, size);
  return send_packet(std::move(packet));
}

// NOLINTNEXTLINE(*-rvalue-reference-param-not-moved): Moved via release_packet
Write_error Client::send_message(Message&& message) {
  if (message.payload_size() == 0)
    return Write_error::empty_buffer;
  if (!message.payload_data())
    return Write_error::null_buffer;

  auto packet = message.release_packet();
  return send_packet(std::move(packet));
}

void Client::send_logout_request() {
  for (auto& connection : connections_)
    (void)connection.send_logout_request();
}

void Client::send_debug(std::string_view text) {
  for (auto& connection : connections_)
    (void)connection.send_debug(text);
}

expected<Connection*, std::error_code>
Client::add_connection(const asio::ip::tcp::endpoint& endpoint,
                       Connection_handler* handler) {
  for (const auto& connection : connections_) {
    if (endpoint == connection.endpoint())
      return unexpected(Error::endpoint_in_use);
  }
  return &connections_.emplace_back(io_executor_, endpoint, *this, handler);
}

Write_error Client::send_packet(Write_packet&& packet) {
  if (has_session_ended_)
    return Write_error::session_ended;

  if (connections_.size() == 1)
    return send_one(std::move(packet));
  if (connections_.size() == 2)
    return send_two(std::move(packet));
  return send_multiple(std::move(packet));
}

Write_error Client::send_one(Write_packet&& packet) {
  assert(connections_.size() == 1);
  auto& connection = connections_.front();
  return connection.send_packet(std::move(packet));
}

namespace {

template <typename Container>
Write_error priority_error(const Container& errors) {
  if (std::ranges::contains(errors, Write_error::none))
    return Write_error::none;
  if (std::ranges::contains(errors, Write_error::buffer_full))
    return Write_error::buffer_full;
  if (std::ranges::contains(errors, Write_error::not_logged_in))
    return Write_error::not_logged_in;
  return Write_error::disconnected;
}

} // namespace

Write_error Client::send_two(Write_packet&& packet) {
  assert(connections_.size() == 2);
  auto& connection1 = connections_.front();
  auto& connection2 = connections_.back();
  Write_packet copy(packet.packet_type(), packet.payload_data(),
                    packet.payload_size());

  const auto error1 = connection1.send_packet(std::move(packet));
  const auto error2 = connection2.send_packet(std::move(copy));
  const std::array<Write_error, 2> errors = {error1, error2};
  return priority_error(errors);
}

Write_error Client::send_multiple(Write_packet&& packet) {
  assert(connections_.empty() || connections_.size() > 2);
  if (connections_.empty())
    return Write_error::disconnected;

  std::vector<Write_error> errors;
  errors.reserve(connections_.size());

  const auto back_iter = std::prev(connections_.end());
  for (auto iter = connections_.begin(); iter != back_iter; ++iter) {
    auto& connection = *iter;
    Write_packet copy(packet.packet_type(), packet.payload_data(),
                      packet.payload_size());
    const auto error = connection.send_packet(std::move(copy));
    errors.push_back(error);
  }
  const auto error = connections_.back().send_packet(std::move(packet));
  errors.push_back(error);

  return priority_error(errors);
}

void Client::on_sequenced_data(std::uint64_t sequence_number, const void* data,
                               std::size_t size) {
  if (sequence_number < next_sequence_number_)
    return;
  assert(sequence_number == next_sequence_number_);
  next_sequence_number_ = sequence_number + 1;
  handler_->sequenced_data(sequence_number, data, size);
}

void Client::on_end_of_session() {
  if (has_session_ended_)
    return;
  has_session_ended_ = true;
  handler_->end_of_session();
}

} // namespace bc::soup::client
