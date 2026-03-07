#include "bc/soup/constants.h"

#include <gtest/gtest.h>

using namespace bc::soup;

TEST(constants, header_lengths) {
  ASSERT_EQ(packet_size_length, 2u);
  ASSERT_EQ(packet_type_length, 1u);
  ASSERT_EQ(packet_header_length, 3u);
}

TEST(constants, field_lengths) {
  ASSERT_EQ(username_length, 6u);
  ASSERT_EQ(password_length, 10u);
  ASSERT_EQ(session_length, 10u);
  ASSERT_EQ(sequence_number_length, 20u);
}

TEST(constants, timeouts) {
  ASSERT_EQ(heartbeat_period.count(), 1);
  ASSERT_EQ(client_heartbeat_timeout.count(), 15);
  ASSERT_EQ(server_heartbeat_timeout.count(), 5);
  ASSERT_EQ(login_request_timeout.count(), 30);
  ASSERT_EQ(login_response_timeout.count(), 10);
}
