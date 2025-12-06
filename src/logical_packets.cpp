#include "bc/soup/logical_packets.h"

#include "bc/soup/packing.h"

#include <cassert>
#include <span>

namespace bc::soup {

Login_accepted_packet::Login_accepted_packet(
    std::string_view session_, std::uint64_t next_sequence_number_)
    : session(session_), next_sequence_number(next_sequence_number_) {
}

void read(Login_accepted_packet& packet, const void* data) {
  constexpr auto size = Login_accepted_packet::payload_size;
  const std::span<const std::byte, size> s(static_cast<const std::byte*>(data),
                                           size);
  auto iter = s.begin();
  unpack_session(packet.session, &*iter);
  iter += session_length;
  unpack_sequence_number(packet.next_sequence_number, &*iter);
  assert((iter += sequence_number_length) == s.end());
}

void write(const Login_accepted_packet& packet, void* data) {
  constexpr auto size = Login_accepted_packet::payload_size;
  const std::span<std::byte, size> s(static_cast<std::byte*>(data), size);
  auto iter = s.begin();
  pack_session(packet.session, &*iter);
  iter += session_length;
  pack_sequence_number(packet.next_sequence_number, &*iter);
  assert((iter += sequence_number_length) == s.end());
}

Login_rejected_packet::Login_rejected_packet(Reason reason_) : reason(reason_) {
}

void read(Login_rejected_packet& packet, const void* data) {
  constexpr auto size = Login_rejected_packet::payload_size;
  const std::span<const std::byte, size> s(static_cast<const std::byte*>(data),
                                           size);
  auto iter = s.begin();
  unpack(packet.reason, &*iter);
  assert((iter += sizeof(Login_rejected_packet::Reason)) == s.end());
}

void write(const Login_rejected_packet& packet, void* data) {
  constexpr auto size = Login_rejected_packet::payload_size;
  const std::span<std::byte, size> s(static_cast<std::byte*>(data), size);
  auto iter = s.begin();
  pack(packet.reason, &*iter);
  assert((iter += sizeof(Login_rejected_packet::Reason)) == s.end());
}

Login_request_packet::Login_request_packet(
    std::string_view username_, std::string_view password_,
    std::string_view requested_session_,
    std::uint64_t requested_sequence_number_)
    : username(username_),
      password(password_),
      requested_session(requested_session_),
      requested_sequence_number(requested_sequence_number_) {
}

void read(Login_request_packet& packet, const void* data) {
  constexpr auto size = Login_request_packet::payload_size;
  const std::span<const std::byte, size> s(static_cast<const std::byte*>(data),
                                           size);
  auto iter = s.begin();
  unpack_username(packet.username, &*iter);
  iter += username_length;
  unpack_password(packet.password, &*iter);
  iter += password_length;
  unpack_session(packet.requested_session, &*iter);
  iter += session_length;
  unpack_sequence_number(packet.requested_sequence_number, &*iter);
  assert((iter += sequence_number_length) == s.end());
}

void write(const Login_request_packet& packet, void* data) {
  constexpr auto size = Login_request_packet::payload_size;
  const std::span<std::byte, size> s(static_cast<std::byte*>(data), size);
  auto iter = s.begin();
  pack_username(packet.username, &*iter);
  iter += username_length;
  pack_password(packet.password, &*iter);
  iter += password_length;
  pack_session(packet.requested_session, &*iter);
  iter += session_length;
  pack_sequence_number(packet.requested_sequence_number, &*iter);
  assert((iter += sequence_number_length) == s.end());
}

} // namespace bc::soup
