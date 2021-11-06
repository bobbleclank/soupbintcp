#ifndef INCLUDE_BC_SOUP_SERVER_MESSAGE_H
#define INCLUDE_BC_SOUP_SERVER_MESSAGE_H

#include "bc/soup/logical_packets.h"
#include "bc/soup/rw_packets.h"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace bc::soup::server {

class Message {
public:
  using Resize_result = Write_packet::Resize_result;

  Message() = default;

  explicit Message(std::uint16_t payload_size)
      : packet_(packet_type_, payload_size) {}

  Message(const void* payload_data, std::uint16_t payload_size)
      : packet_(packet_type_, payload_data, payload_size) {}

  ~Message() = default;

  Message(const Message&) = delete;
  Message& operator=(const Message&) = delete;

  Message(Message&&) = default;
  Message& operator=(Message&&) = default;

  Write_packet release_packet() { return std::move(packet_); }

  [[nodiscard]] Resize_result resize_payload(std::uint16_t payload_size) {
    return packet_.resize_payload(payload_size);
  }

  void* payload_data() { return packet_.payload_data(); }

  const void* payload_data() const { return packet_.payload_data(); }

  std::size_t payload_size() const { return packet_.payload_size(); }

  std::size_t payload_capacity() const { return packet_.payload_capacity(); }

private:
  static constexpr char packet_type_ = Sequenced_data_packet::packet_type;
  Write_packet packet_;
};

} // namespace bc::soup::server

#endif
