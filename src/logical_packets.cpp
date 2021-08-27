#include "soup/logical_packets.h"

#include "soup/packing.h"

namespace soup {

Login_accepted_packet::Login_accepted_packet(
    std::string_view session_, std::uint64_t next_sequence_number_)
    : session(session_), next_sequence_number(next_sequence_number_) {}

void read(Login_accepted_packet& packet, const void* data) {
  auto* ptr = static_cast<const std::byte*>(data);
  unpack_session(packet.session, ptr);
  ptr += session_length;
  unpack_sequence_number(packet.next_sequence_number, ptr);
}

void write(const Login_accepted_packet& packet, void* data) {
  auto* ptr = static_cast<std::byte*>(data);
  pack_session(packet.session, ptr);
  ptr += session_length;
  pack_sequence_number(packet.next_sequence_number, ptr);
}

Login_rejected_packet::Login_rejected_packet(Reason reason_)
    : reason(reason_) {}

void read(Login_rejected_packet& packet, const void* data) {
  auto* ptr = static_cast<const std::byte*>(data);
  unpack(packet.reason, ptr);
}

void write(const Login_rejected_packet& packet, void* data) {
  auto* ptr = static_cast<std::byte*>(data);
  pack(packet.reason, ptr);
}

Login_request_packet::Login_request_packet(
    std::string_view username_, std::string_view password_,
    std::string_view requested_session_,
    std::uint64_t requested_sequence_number_)
    : username(username_), password(password_),
      requested_session(requested_session_),
      requested_sequence_number(requested_sequence_number_) {}

void read(Login_request_packet& packet, const void* data) {
  auto* ptr = static_cast<const std::byte*>(data);
  unpack_username(packet.username, ptr);
  ptr += username_length;
  unpack_password(packet.password, ptr);
  ptr += password_length;
  unpack_session(packet.requested_session, ptr);
  ptr += session_length;
  unpack_sequence_number(packet.requested_sequence_number, ptr);
}

void write(const Login_request_packet& packet, void* data) {
  auto* ptr = static_cast<std::byte*>(data);
  pack_username(packet.username, ptr);
  ptr += username_length;
  pack_password(packet.password, ptr);
  ptr += password_length;
  pack_session(packet.requested_session, ptr);
  ptr += session_length;
  pack_sequence_number(packet.requested_sequence_number, ptr);
}

} // namespace soup
