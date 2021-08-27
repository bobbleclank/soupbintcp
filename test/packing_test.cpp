#include "soup/packing.h"

#include <cstdint>

#include <gtest/gtest.h>

using namespace soup;

TEST(packing, integral) {
  {
    char i = 'a';
    unsigned char b[sizeof(i)];
    pack(i, b);
    ASSERT_EQ(*reinterpret_cast<char*>(b), 0X61);
    char j = '\0';
    unpack(j, b);
    ASSERT_EQ(j, 'a');
  }
  {
    std::int16_t i = 21;
    unsigned char b[sizeof(i)];
    pack(i, b);
    ASSERT_EQ(*reinterpret_cast<std::int16_t*>(b), 0X1500);
    std::int16_t j = 0;
    unpack(j, b);
    ASSERT_EQ(j, 21);
  }
  {
    std::uint16_t i = 21;
    unsigned char b[sizeof(i)];
    pack(i, b);
    ASSERT_EQ(*reinterpret_cast<std::int16_t*>(b), 0X1500);
    std::uint16_t j = 0;
    unpack(j, b);
    ASSERT_EQ(j, 21);
  }
  {
    std::int32_t i = 42;
    unsigned char b[sizeof(i)];
    pack(i, b);
    ASSERT_EQ(*reinterpret_cast<std::int32_t*>(b), 0X2A000000);
    std::int32_t j = 0;
    unpack(j, b);
    ASSERT_EQ(j, 42);
  }
  {
    std::uint32_t i = 42;
    unsigned char b[sizeof(i)];
    pack(i, b);
    ASSERT_EQ(*reinterpret_cast<std::int32_t*>(b), 0X2A000000);
    std::uint32_t j = 0;
    unpack(j, b);
    ASSERT_EQ(j, 42);
  }
}

namespace {

enum class E1 : char { a = 'a', b = 'b', c = 'c' };

enum class E2 : std::uint16_t { a = 21, b = 42, c = 84 };

enum class E3 : std::uint32_t { a = 21, b = 42, c = 84 };

} // namespace

TEST(packing, enumeration) {
  {
    E1 i = E1::c;
    unsigned char b[sizeof(i)];
    pack(i, b);
    ASSERT_EQ(*reinterpret_cast<char*>(b), 0X63);
    E1 j = E1::a;
    unpack(j, b);
    ASSERT_EQ(j, E1::c);
  }
  {
    E2 i = E2::c;
    unsigned char b[sizeof(i)];
    pack(i, b);
    ASSERT_EQ(*reinterpret_cast<std::int16_t*>(b), 0X5400);
    E2 j = E2::a;
    unpack(j, b);
    ASSERT_EQ(j, E2::c);
  }
  {
    E3 i = E3::c;
    unsigned char b[sizeof(i)];
    pack(i, b);
    ASSERT_EQ(*reinterpret_cast<std::int32_t*>(b), 0X54000000);
    E3 j = E3::a;
    unpack(j, b);
    ASSERT_EQ(j, E3::c);
  }
}
