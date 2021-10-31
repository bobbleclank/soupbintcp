#include "bc/soup/constants.h"

#include <gtest/gtest.h>

using namespace bc::soup;

TEST(constants, header_lengths) {
  ASSERT_EQ(packet_size_length, 2);
  ASSERT_EQ(packet_type_length, 1);
  ASSERT_EQ(packet_header_length, 3);
}

TEST(constants, field_lengths) {
  ASSERT_EQ(username_length, 6);
  ASSERT_EQ(password_length, 10);
  ASSERT_EQ(session_length, 10);
  ASSERT_EQ(sequence_number_length, 20);
}
