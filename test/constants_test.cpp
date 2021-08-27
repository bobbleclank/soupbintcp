#include "soup/constants.h"

#include <gtest/gtest.h>

using namespace soup;

TEST(constants, header_lengths) {
  ASSERT_EQ(packet_size_length, 2);
  ASSERT_EQ(packet_type_length, 1);
  ASSERT_EQ(packet_header_length, 3);
}
