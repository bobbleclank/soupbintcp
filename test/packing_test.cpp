#include "bc/soup/packing.h"

#include <array>
#include <cstdint>
#include <limits>
#include <string>

#include <gtest/gtest.h>

using namespace bc::soup;
using namespace bc::soup::internal;

using namespace std::string_literals;

TEST(packing, integral) {
  {
    char i = 'a';
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    ASSERT_EQ(*reinterpret_cast<char*>(b.data()), 0X61);
    char j = '\0';
    unpack(j, b.data());
    ASSERT_EQ(j, 'a');
  }
  {
    std::int16_t i = 21;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    ASSERT_EQ(*reinterpret_cast<std::int16_t*>(b.data()), 0X1500);
    std::int16_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 21);
  }
  {
    std::uint16_t i = 21;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    ASSERT_EQ(*reinterpret_cast<std::int16_t*>(b.data()), 0X1500);
    std::uint16_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 21u);
  }
  {
    std::int32_t i = 42;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    ASSERT_EQ(*reinterpret_cast<std::int32_t*>(b.data()), 0X2A000000);
    std::int32_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 42);
  }
  {
    std::uint32_t i = 42;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    ASSERT_EQ(*reinterpret_cast<std::int32_t*>(b.data()), 0X2A000000);
    std::uint32_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 42u);
  }
}

namespace {

enum class E1 : char {
  a = 'a',
  b = 'b',
  c = 'c'
};

enum class E2 : std::uint16_t {
  a = 21,
  b = 42,
  c = 84
};

enum class E3 : std::uint32_t {
  a = 21,
  b = 42,
  c = 84
};

} // namespace

TEST(packing, enumeration) {
  {
    E1 i = E1::c;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    ASSERT_EQ(*reinterpret_cast<char*>(b.data()), 0X63);
    E1 j = E1::a;
    unpack(j, b.data());
    ASSERT_EQ(j, E1::c);
  }
  {
    E2 i = E2::c;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    ASSERT_EQ(*reinterpret_cast<std::int16_t*>(b.data()), 0X5400);
    E2 j = E2::a;
    unpack(j, b.data());
    ASSERT_EQ(j, E2::c);
  }
  {
    E3 i = E3::c;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    ASSERT_EQ(*reinterpret_cast<std::int32_t*>(b.data()), 0X54000000);
    E3 j = E3::a;
    unpack(j, b.data());
    ASSERT_EQ(j, E3::c);
  }
}

TEST(packing, pack_alphanumeric) {
  {
    std::string s = "";
    std::array<char, 4> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), "    ", b.size()), 0);
  }
  {
    std::string s = "a";
    std::array<char, 4> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), "a   ", b.size()), 0);
  }
  {
    std::string s = "ab";
    std::array<char, 4> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), "ab  ", b.size()), 0);
  }
  {
    std::string s = "abc";
    std::array<char, 4> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), "abc ", b.size()), 0);
  }
  {
    std::string s = "abcd";
    std::array<char, 4> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), "abcd", b.size()), 0);
  }
  {
    std::string s = "  ab"; // Leading invalid character.
    std::array<char, 4> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), "    ", b.size()), 0);
  }
  {
    std::string s = "ab__"; // Trailing invalid character.
    std::array<char, 4> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), "ab  ", b.size()), 0);
  }
  {
    std::string s = "a  b"; // Embedded invalid character.
    std::array<char, 4> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), "a   ", b.size()), 0);
  }
  {
    std::string s = "abcde"; // Too long.
    std::array<char, 4> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(std::memcmp(b.data(), "abcd", b.size()), 0);
  }
}

TEST(packing, unpack_alphanumeric) {
  {
    std::string b = "    ";
    ASSERT_EQ(b.size(), 4);
    std::string s;
    unpack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string b = "a   ";
    ASSERT_EQ(b.size(), 4);
    std::string s;
    unpack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(s, "a"s);
  }
  {
    std::string b = "ab  ";
    ASSERT_EQ(b.size(), 4);
    std::string s;
    unpack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(s, "ab"s);
  }
  {
    std::string b = "abc ";
    ASSERT_EQ(b.size(), 4);
    std::string s;
    unpack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(s, "abc"s);
  }
  {
    std::string b = "abcd";
    ASSERT_EQ(b.size(), 4);
    std::string s;
    unpack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(s, "abcd"s);
  }
  {
    std::string b = "  ab"; // Leading invalid character.
    ASSERT_EQ(b.size(), 4);
    std::string s;
    unpack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string b = "ab__"; // Trailing invalid character.
    ASSERT_EQ(b.size(), 4);
    std::string s;
    unpack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(s, "ab"s);
  }
  {
    std::string b = "a  b"; // Embedded invalid character.
    ASSERT_EQ(b.size(), 4);
    std::string s;
    unpack_alphanumeric(s, b.data(), b.size());
    ASSERT_EQ(s, "a"s);
  }
  {
    std::string b = "abcde"; // Too long.
    ASSERT_EQ(b.size(), 4 + 1);
    std::string s;
    unpack_alphanumeric(s, b.data(), b.size() - 1);
    ASSERT_EQ(s, "abcd"s);
  }
}

TEST(packing, pack_session) {
  {
    std::string s = "";
    std::array<char, 10> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "          ", b.size()), 0);
  }
  {
    std::string s = "a";
    std::array<char, 10> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "         a", b.size()), 0);
  }
  {
    std::string s = "abcde";
    std::array<char, 10> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "     abcde", b.size()), 0);
  }
  {
    std::string s = "abcdefghi";
    std::array<char, 10> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), " abcdefghi", b.size()), 0);
  }
  {
    std::string s = "abcdefghij";
    std::array<char, 10> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "abcdefghij", b.size()), 0);
  }
  {
    std::string s = "     abcde"; // Leading invalid character.
    std::array<char, 10> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "          ", b.size()), 0);
  }
  {
    std::string s = "abcde_____"; // Trailing invalid character.
    std::array<char, 10> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "     abcde", b.size()), 0);
  }
  {
    std::string s = "abc    def"; // Embedded invalid character.
    std::array<char, 10> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "       abc", b.size()), 0);
  }
  {
    std::string s = "abcdefghijk"; // Too long.
    std::array<char, 10> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "abcdefghij", b.size()), 0);
  }
}

TEST(packing, unpack_session) {
  {
    std::string b = "          ";
    ASSERT_EQ(b.size(), 10);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string b = "         a";
    ASSERT_EQ(b.size(), 10);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "a"s);
  }
  {
    std::string b = "     abcde";
    ASSERT_EQ(b.size(), 10);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcde"s);
  }
  {
    std::string b = " abcdefghi";
    ASSERT_EQ(b.size(), 10);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcdefghi"s);
  }
  {
    std::string b = "abcdefghij";
    ASSERT_EQ(b.size(), 10);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcdefghij"s);
  }
  {
    std::string b = "  ---abcde"; // Leading invalid character.
    ASSERT_EQ(b.size(), 10);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string b = "abcde_____"; // Trailing invalid character.
    ASSERT_EQ(b.size(), 10);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcde"s);
  }
  {
    std::string b = "abc    def"; // Embedded invalid character.
    ASSERT_EQ(b.size(), 10);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abc"s);
  }
  {
    std::string b = "abcdefghijk"; // Too long.
    ASSERT_EQ(b.size(), 10 + 1);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcdefghij"s);
  }
}

TEST(packing, pack_sequence_number) {
  {
    std::uint64_t i = 0;
    std::array<char, 20> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "                   0", b.size()), 0);
  }
  {
    std::uint64_t i = 1;
    std::array<char, 20> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "                   1", b.size()), 0);
  }
  {
    std::uint64_t i = 12345;
    std::array<char, 20> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "               12345", b.size()), 0);
  }
  {
    std::uint64_t i = 1234567890;
    std::array<char, 20> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "          1234567890", b.size()), 0);
  }
  {
    std::uint64_t i = 9999999999999999999uL;
    std::array<char, 20> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), " 9999999999999999999", b.size()), 0);
  }
  {
    std::uint64_t i = std::numeric_limits<std::uint64_t>::max();
    std::array<char, 20> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "18446744073709551615", b.size()), 0);
  }
}

TEST(packing, unpack_sequence_number) {
  {
    std::string b = "                   0";
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string b = "                   1";
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1u);
  }
  {
    std::string b = "               12345";
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 12345u);
  }
  {
    std::string b = "          1234567890";
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1234567890u);
  }
  {
    std::string b = " 9999999999999999999";
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 9999999999999999999uL);
  }
  {
    std::string b = "18446744073709551615";
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, std::numeric_limits<std::uint64_t>::max());
  }
  {
    std::string b = "                    "; // No digits.
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string b = "          123 567890"; // Non-digit.
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 123u);
  }
  {
    std::string b = "18446744073709551616"; // Overflow.
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string b = "18446744073709551617"; // Overflow.
    ASSERT_EQ(b.size(), 20);
    std::uint64_t i = 42;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1u);
  }
}
