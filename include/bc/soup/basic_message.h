#ifndef INCLUDE_BC_SOUP_BASIC_MESSAGE_H
#define INCLUDE_BC_SOUP_BASIC_MESSAGE_H

#include "bc/soup/rw_packets.h"

#include <cstddef>
#include <utility>

namespace bc::soup {

template <typename Packet>
class Basic_message {
public:
  using Resize_result = Write_packet::Resize_result;

  Basic_message() = default;

  explicit Basic_message(std::size_t payload_size)
      : packet_(packet_type_, payload_size) {}

  Basic_message(const void* payload_data, std::size_t payload_size)
      : packet_(packet_type_, payload_data, payload_size) {}

  ~Basic_message() = default;

  Basic_message(const Basic_message&) = delete;
  Basic_message& operator=(const Basic_message&) = delete;

  Basic_message(Basic_message&&) = default;
  Basic_message& operator=(Basic_message&&) = default;

  Write_packet release_packet() { return std::move(packet_); }

  [[nodiscard]] Resize_result resize_payload(std::size_t payload_size) {
    return packet_.resize_payload(payload_size);
  }

  void* payload_data() { return packet_.payload_data(); }

  const void* payload_data() const { return packet_.payload_data(); }

  std::size_t payload_size() const { return packet_.payload_size(); }

  std::size_t payload_capacity() const { return packet_.payload_capacity(); }

private:
  static constexpr char packet_type_ = Packet::packet_type;
  Write_packet packet_;
};

} // namespace bc::soup

#endif
