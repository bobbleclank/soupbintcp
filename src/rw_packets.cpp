#include "soup/rw_packets.h"

#include "soup/packing.h"

#include <cstring>
#include <utility>

namespace soup {

Buffer::Buffer(std::size_t size)
    : data_(std::make_unique<std::byte[]>(size)), size_(size) {}

Buffer::Buffer(Buffer&& other) noexcept
    : data_(std::move(other.data_)), size_(other.size_) {
  other.size_ = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
  data_ = std::move(other.data_);
  size_ = other.size_;
  other.size_ = 0;
  return *this;
}

Read_packet::Read_packet() { header_.fill(std::byte(0)); }

Read_packet::Read_packet(Read_packet&& other) noexcept
    : header_(other.header_), payload_(std::move(other.payload_)) {
  other.header_.fill(std::byte(0));
}

Read_packet& Read_packet::operator=(Read_packet&& other) noexcept {
  header_ = other.header_;
  payload_ = std::move(other.payload_);
  other.header_.fill(std::byte(0));
  return *this;
}

std::uint16_t Read_packet::packet_size() const {
  std::uint16_t size = 0;
  unpack(size, header_.data());
  return size;
}

Read_packet::Resize_result Read_packet::resize_payload() {
  payload_ = Buffer();
  auto packet_size = this->packet_size();
  if (packet_size < packet_type_length)
    return Resize_result::bad_packet;
  if (packet_size == packet_type_length)
    return Resize_result::empty_payload;
  payload_ = Buffer(packet_size - packet_type_length);
  return Resize_result::resized;
}

Write_packet::Write_packet(char packet_type) : Write_packet(packet_type, 0) {}

Write_packet::Write_packet(char packet_type, std::uint16_t payload_size)
    : packet_(packet_header_length + payload_size) {
  std::uint16_t packet_size = packet_type_length + payload_size;
  pack(packet_size, packet_.data());
  packet_[packet_size_length] = static_cast<std::byte>(packet_type);
}

Write_packet::Write_packet(char packet_type, const void* payload_data,
                           std::uint16_t payload_size)
    : Write_packet(packet_type, payload_size) {
  std::memcpy(packet_.data() + packet_header_length, payload_data,
              payload_size);
}

std::uint16_t Write_packet::packet_size() const {
  std::uint16_t size = 0;
  unpack(size, packet_.data());
  return size;
}

Write_packet::Resize_result
Write_packet::resize_payload(std::uint16_t payload_size) {
  if (payload_size > payload_capacity())
    return Resize_result::no_capacity;
  std::uint16_t packet_size = packet_type_length + payload_size;
  pack(packet_size, packet_.data());
  return Resize_result::resized;
}

} // namespace soup
