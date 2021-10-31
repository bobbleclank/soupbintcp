#include "bc/soup/packing.h"

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

TEST(packing, pack_alphanumeric) {
  {
    const char s[] = "";
    char b[4];
    std::memset(b, '*', sizeof(b));
    pack_alphanumeric(s, b, 4);
    ASSERT_EQ(std::memcmp(b, "    ", 4), 0);
  }
  {
    const char s[] = "a";
    char b[4];
    std::memset(b, '*', sizeof(b));
    pack_alphanumeric(s, b, 4);
    ASSERT_EQ(std::memcmp(b, "a   ", 4), 0);
  }
  {
    const char s[] = "ab";
    char b[4];
    std::memset(b, '*', sizeof(b));
    pack_alphanumeric(s, b, 4);
    ASSERT_EQ(std::memcmp(b, "ab  ", 4), 0);
  }
  {
    const char s[] = "abc";
    char b[4];
    std::memset(b, '*', sizeof(b));
    pack_alphanumeric(s, b, 4);
    ASSERT_EQ(std::memcmp(b, "abc ", 4), 0);
  }
  {
    const char s[] = "abcd";
    char b[4];
    std::memset(b, '*', sizeof(b));
    pack_alphanumeric(s, b, 4);
    ASSERT_EQ(std::memcmp(b, "abcd", 4), 0);
  }
  {
    const char s[] = "  ab"; // Leading invalid character.
    char b[4];
    std::memset(b, '*', sizeof(b));
    pack_alphanumeric(s, b, 4);
    ASSERT_EQ(std::memcmp(b, "    ", 4), 0);
  }
  {
    const char s[] = "ab__"; // Trailing invalid character.
    char b[4];
    std::memset(b, '*', sizeof(b));
    pack_alphanumeric(s, b, 4);
    ASSERT_EQ(std::memcmp(b, "ab  ", 4), 0);
  }
  {
    const char s[] = "a  b"; // Embedded invalid character.
    char b[4];
    std::memset(b, '*', sizeof(b));
    pack_alphanumeric(s, b, 4);
    ASSERT_EQ(std::memcmp(b, "a   ", 4), 0);
  }
  {
    const char s[] = "abcde"; // Too long.
    char b[4];
    std::memset(b, '*', sizeof(b));
    pack_alphanumeric(s, b, 4);
    ASSERT_EQ(std::memcmp(b, "abcd", 4), 0);
  }
}

TEST(packing, unpack_alphanumeric) {
  {
    char b[4 + 1] = "    ";
    std::string s;
    unpack_alphanumeric(s, b, 4);
    ASSERT_EQ(s, ""s);
  }
  {
    char b[4 + 1] = "a   ";
    std::string s;
    unpack_alphanumeric(s, b, 4);
    ASSERT_EQ(s, "a"s);
  }
  {
    char b[4 + 1] = "ab  ";
    std::string s;
    unpack_alphanumeric(s, b, 4);
    ASSERT_EQ(s, "ab"s);
  }
  {
    char b[4 + 1] = "abc ";
    std::string s;
    unpack_alphanumeric(s, b, 4);
    ASSERT_EQ(s, "abc"s);
  }
  {
    char b[4 + 1] = "abcd";
    std::string s;
    unpack_alphanumeric(s, b, 4);
    ASSERT_EQ(s, "abcd"s);
  }
  {
    char b[4 + 1] = "  ab"; // Leading invalid character.
    std::string s;
    unpack_alphanumeric(s, b, 4);
    ASSERT_EQ(s, ""s);
  }
  {
    char b[4 + 1] = "ab__"; // Trailing invalid character.
    std::string s;
    unpack_alphanumeric(s, b, 4);
    ASSERT_EQ(s, "ab"s);
  }
  {
    char b[4 + 1] = "a  b"; // Embedded invalid character.
    std::string s;
    unpack_alphanumeric(s, b, 4);
    ASSERT_EQ(s, "a"s);
  }
  {
    char b[4 + 1 + 1] = "abcde"; // Too long.
    std::string s;
    unpack_alphanumeric(s, b, 4);
    ASSERT_EQ(s, "abcd"s);
  }
}

TEST(packing, pack_session) {
  {
    const char s[] = "";
    char b[10];
    std::memset(b, '*', sizeof(b));
    pack_session(s, b);
    ASSERT_EQ(std::memcmp(b, "          ", 10), 0);
  }
  {
    const char s[] = "a";
    char b[10];
    std::memset(b, '*', sizeof(b));
    pack_session(s, b);
    ASSERT_EQ(std::memcmp(b, "         a", 10), 0);
  }
  {
    const char s[] = "abcde";
    char b[10];
    std::memset(b, '*', sizeof(b));
    pack_session(s, b);
    ASSERT_EQ(std::memcmp(b, "     abcde", 10), 0);
  }
  {
    const char s[] = "abcdefghi";
    char b[10];
    std::memset(b, '*', sizeof(b));
    pack_session(s, b);
    ASSERT_EQ(std::memcmp(b, " abcdefghi", 10), 0);
  }
  {
    const char s[] = "abcdefghij";
    char b[10];
    std::memset(b, '*', sizeof(b));
    pack_session(s, b);
    ASSERT_EQ(std::memcmp(b, "abcdefghij", 10), 0);
  }
  {
    const char s[] = "     abcde"; // Leading invalid character.
    char b[10];
    std::memset(b, '*', sizeof(b));
    pack_session(s, b);
    ASSERT_EQ(std::memcmp(b, "          ", 10), 0);
  }
  {
    const char s[] = "abcde_____"; // Trailing invalid character.
    char b[10];
    std::memset(b, '*', sizeof(b));
    pack_session(s, b);
    ASSERT_EQ(std::memcmp(b, "     abcde", 10), 0);
  }
  {
    const char s[] = "abc    def"; // Embedded invalid character.
    char b[10];
    std::memset(b, '*', sizeof(b));
    pack_session(s, b);
    ASSERT_EQ(std::memcmp(b, "       abc", 10), 0);
  }
  {
    const char s[] = "abcdefghijk"; // Too long.
    char b[10];
    std::memset(b, '*', sizeof(b));
    pack_session(s, b);
    ASSERT_EQ(std::memcmp(b, "abcdefghij", 10), 0);
  }
}

TEST(packing, unpack_session) {
  {
    char b[10 + 1] = "          ";
    std::string s;
    unpack_session(s, b);
    ASSERT_EQ(s, ""s);
  }
  {
    char b[10 + 1] = "         a";
    std::string s;
    unpack_session(s, b);
    ASSERT_EQ(s, "a"s);
  }
  {
    char b[10 + 1] = "     abcde";
    std::string s;
    unpack_session(s, b);
    ASSERT_EQ(s, "abcde"s);
  }
  {
    char b[10 + 1] = " abcdefghi";
    std::string s;
    unpack_session(s, b);
    ASSERT_EQ(s, "abcdefghi"s);
  }
  {
    char b[10 + 1] = "abcdefghij";
    std::string s;
    unpack_session(s, b);
    ASSERT_EQ(s, "abcdefghij"s);
  }
  {
    char b[10 + 1] = "  ---abcde"; // Leading invalid character.
    std::string s;
    unpack_session(s, b);
    ASSERT_EQ(s, ""s);
  }
  {
    char b[10 + 1] = "abcde_____"; // Trailing invalid character.
    std::string s;
    unpack_session(s, b);
    ASSERT_EQ(s, "abcde"s);
  }
  {
    char b[10 + 1] = "abc    def"; // Embedded invalid character.
    std::string s;
    unpack_session(s, b);
    ASSERT_EQ(s, "abc"s);
  }
  {
    char b[10 + 1 + 1] = "abcdefghijk"; // Too long.
    std::string s;
    unpack_session(s, b);
    ASSERT_EQ(s, "abcdefghij"s);
  }
}

TEST(packing, pack_sequence_number) {
  {
    std::uint64_t i = 0;
    char b[20];
    std::memset(b, '*', sizeof(b));
    pack_sequence_number(i, b);
    ASSERT_EQ(std::memcmp(b, "                   0", 20), 0);
  }
  {
    std::uint64_t i = 1;
    char b[20];
    std::memset(b, '*', sizeof(b));
    pack_sequence_number(i, b);
    ASSERT_EQ(std::memcmp(b, "                   1", 20), 0);
  }
  {
    std::uint64_t i = 12345;
    char b[20];
    std::memset(b, '*', sizeof(b));
    pack_sequence_number(i, b);
    ASSERT_EQ(std::memcmp(b, "               12345", 20), 0);
  }
  {
    std::uint64_t i = 1234567890;
    char b[20];
    std::memset(b, '*', sizeof(b));
    pack_sequence_number(i, b);
    ASSERT_EQ(std::memcmp(b, "          1234567890", 20), 0);
  }
  {
    std::uint64_t i = 9999999999999999999uL;
    char b[20];
    std::memset(b, '*', sizeof(b));
    pack_sequence_number(i, b);
    ASSERT_EQ(std::memcmp(b, " 9999999999999999999", 20), 0);
  }
  {
    std::uint64_t i = std::numeric_limits<std::uint64_t>::max();
    char b[20];
    std::memset(b, '*', sizeof(b));
    pack_sequence_number(i, b);
    ASSERT_EQ(std::memcmp(b, "18446744073709551615", 20), 0);
  }
}

TEST(packing, unpack_sequence_number) {
  {
    char b[20 + 1] = "                   0";
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, 0);
  }
  {
    char b[20 + 1] = "                   1";
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, 1);
  }
  {
    char b[20 + 1] = "               12345";
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, 12345);
  }
  {
    char b[20 + 1] = "          1234567890";
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, 1234567890);
  }
  {
    char b[20 + 1] = " 9999999999999999999";
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, 9999999999999999999uL);
  }
  {
    char b[20 + 1] = "18446744073709551615";
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, std::numeric_limits<std::uint64_t>::max());
  }
  {
    char b[20 + 1] = "                    "; // No digits.
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, 0);
  }
  {
    char b[20 + 1] = "          123 567890"; // Non-digit.
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, 123);
  }
  {
    char b[20 + 1] = "18446744073709551616"; // Overflow.
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, 0);
  }
  {
    char b[20 + 1] = "18446744073709551617"; // Overflow.
    std::uint64_t i = 42;
    unpack_sequence_number(i, b);
    ASSERT_EQ(i, 1);
  }
}
