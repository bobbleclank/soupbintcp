#ifndef INCLUDE_BC_SOUP_RW_PACKETS_H
#define INCLUDE_BC_SOUP_RW_PACKETS_H

#include "bc/soup/constants.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace bc::soup {

class Buffer {
public:
  Buffer() = default;
  explicit Buffer(std::size_t);
  ~Buffer() = default;

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  Buffer(Buffer&&) noexcept;
  Buffer& operator=(Buffer&&) noexcept;

  std::byte* data() { return data_.get(); }

  const std::byte* data() const { return data_.get(); }

  std::byte& operator[](std::size_t i) { return data_[i]; }

  const std::byte& operator[](std::size_t i) const { return data_[i]; }

  std::size_t size() const { return size_; }

private:
  std::unique_ptr<std::byte[]> data_;
  std::size_t size_ = 0;
};

class Read_packet {
public:
  enum class Resize_result { resized, empty_payload, bad_packet };

  Read_packet();
  ~Read_packet() = default;

  Read_packet(const Read_packet&) = delete;
  Read_packet& operator=(const Read_packet&) = delete;

  Read_packet(Read_packet&&) noexcept;
  Read_packet& operator=(Read_packet&&) noexcept;

  void* header_data() { return header_.data(); }

  constexpr std::size_t header_size() const { return header_.size(); }

  std::uint16_t packet_size() const;

  char packet_type() const {
    return static_cast<char>(header_[packet_size_length]);
  }

  [[nodiscard]] Resize_result resize_payload();

  void* payload_data() { return payload_.data(); }

  const void* payload_data() const { return payload_.data(); }

  std::size_t payload_size() const { return payload_.size(); }

private:
  std::array<std::byte, packet_header_length> header_;
  Buffer payload_;
};

class Write_packet {
public:
  enum class Resize_result { resized, no_capacity };

  Write_packet() = default;
  explicit Write_packet(char);
  Write_packet(char, std::uint16_t);
  Write_packet(char, const void*, std::uint16_t);
  ~Write_packet() = default;

  Write_packet(const Write_packet&) = delete;
  Write_packet& operator=(const Write_packet&) = delete;

  Write_packet(Write_packet&&) = default;
  Write_packet& operator=(Write_packet&&) = default;

  std::uint16_t packet_size() const;

  char packet_type() const {
    return static_cast<char>(packet_[packet_size_length]);
  }

  [[nodiscard]] Resize_result resize_payload(std::uint16_t);

  void* payload_data() { return packet_.data() + packet_header_length; }

  const void* payload_data() const {
    return packet_.data() + packet_header_length;
  }

  std::size_t payload_size() const {
    return packet_size() - packet_type_length;
  }

  std::size_t payload_capacity() const {
    return packet_.size() - packet_header_length;
  }

  const void* data() const { return packet_.data(); }

  std::size_t size() const { return packet_size_length + packet_size(); }

private:
  Buffer packet_;
};

} // namespace bc::soup

#endif
