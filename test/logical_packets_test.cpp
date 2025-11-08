#include "bc/soup/logical_packets.h"

#include <array>
#include <cstring>
#include <string>

#include <gtest/gtest.h>

using namespace bc::soup;

TEST(logical_packets, packet_type) {
  ASSERT_EQ(Debug_packet::packet_type, '+');

  ASSERT_EQ(Login_accepted_packet::packet_type, 'A');
  ASSERT_EQ(Login_rejected_packet::packet_type, 'J');
  ASSERT_EQ(Sequenced_data_packet::packet_type, 'S');
  ASSERT_EQ(Server_heartbeat_packet::packet_type, 'H');
  ASSERT_EQ(End_of_session_packet::packet_type, 'Z');

  ASSERT_EQ(Login_request_packet::packet_type, 'L');
  ASSERT_EQ(Unsequenced_data_packet::packet_type, 'U');
  ASSERT_EQ(Client_heartbeat_packet::packet_type, 'R');
  ASSERT_EQ(Logout_request_packet::packet_type, 'O');
}

TEST(logical_packets, payload_size) {
  ASSERT_EQ(Debug_packet::payload_size, 0u);

  ASSERT_EQ(Login_accepted_packet::payload_size, 30u);
  ASSERT_EQ(Login_rejected_packet::payload_size, 1u);
  ASSERT_EQ(Server_heartbeat_packet::payload_size, 0u);
  ASSERT_EQ(End_of_session_packet::payload_size, 0u);

  ASSERT_EQ(Login_request_packet::payload_size, 46u);
  ASSERT_EQ(Client_heartbeat_packet::payload_size, 0u);
  ASSERT_EQ(Logout_request_packet::payload_size, 0u);
}

TEST(logical_packets, login_rejected_reason) {
  using Reason = Login_rejected_packet::Reason;
  ASSERT_EQ(static_cast<char>(Reason::not_authorized), 'A');
  ASSERT_EQ(static_cast<char>(Reason::session_not_available), 'S');
  ASSERT_EQ(static_cast<char>(Reason::sequence_number_too_high), 'N');
}

TEST(logical_packets, Login_accepted_packet) {
  {
    Login_accepted_packet p;
    ASSERT_EQ(p.session, "");
    ASSERT_EQ(p.next_sequence_number, 1u);

    std::string b = "     abcde          1234567890";
    ASSERT_EQ(b.size(), Login_accepted_packet::payload_size);
    read(p, b.data());
    ASSERT_EQ(p.session, "abcde");
    ASSERT_EQ(p.next_sequence_number, 1234567890u);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    Login_accepted_packet p("abcde", 1234567890);
    ASSERT_EQ(p.session, "abcde");
    ASSERT_EQ(p.next_sequence_number, 1234567890u);

    std::array<char, Login_accepted_packet::payload_size> b = {};
    b.fill('*');
    write(p, b.data());
    std::string expected = "     abcde          1234567890";
    ASSERT_EQ(expected.size(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), expected.data(), b.size()), 0);
  }
}

TEST(logical_packets, Login_rejected_packet) {
  using Reason = Login_rejected_packet::Reason;
  {
    Login_rejected_packet p;
    ASSERT_EQ(p.reason, Reason::not_authorized);

    std::string b = "S";
    ASSERT_EQ(b.size(), Login_rejected_packet::payload_size);
    read(p, b.data());
    ASSERT_EQ(p.reason, Reason::session_not_available);
  }
  {
    Login_rejected_packet p(Reason::session_not_available);
    ASSERT_EQ(p.reason, Reason::session_not_available);

    std::array<char, Login_rejected_packet::payload_size> b = {};
    b.fill('*');
    write(p, b.data());
    std::string expected = "S";
    ASSERT_EQ(expected.size(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), expected.data(), b.size()), 0);
  }
}

TEST(logical_packets, Login_request_packet) {
  {
    Login_request_packet p;
    ASSERT_EQ(p.username, "");
    ASSERT_EQ(p.password, "");
    ASSERT_EQ(p.requested_session, "");
    ASSERT_EQ(p.requested_sequence_number, 0u);

    std::string b = "ABC   DEFGH          abcde          1234567890";
    ASSERT_EQ(b.size(), Login_request_packet::payload_size);
    read(p, b.data());
    ASSERT_EQ(p.username, "ABC");
    ASSERT_EQ(p.password, "DEFGH");
    ASSERT_EQ(p.requested_session, "abcde");
    ASSERT_EQ(p.requested_sequence_number, 1234567890u);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    Login_request_packet p("ABC", "DEFGH", "abcde", 1234567890);
    ASSERT_EQ(p.username, "ABC");
    ASSERT_EQ(p.password, "DEFGH");
    ASSERT_EQ(p.requested_session, "abcde");
    ASSERT_EQ(p.requested_sequence_number, 1234567890u);

    std::array<char, Login_request_packet::payload_size> b = {};
    b.fill('*');
    write(p, b.data());
    std::string expected = "ABC   DEFGH          abcde          1234567890";
    ASSERT_EQ(expected.size(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), expected.data(), b.size()), 0);
  }
}
