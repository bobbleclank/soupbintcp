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
    constexpr char k = 0X61;
    ASSERT_EQ(std::memcmp(b.data(), &k, b.size()), 0);
    char j = '\0';
    unpack(j, b.data());
    ASSERT_EQ(j, 'a');
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::int16_t i = 4660;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::int16_t k = 0X3412;
    ASSERT_EQ(std::memcmp(b.data(), &k, b.size()), 0);
    std::int16_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 4660);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint16_t i = 4660;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::uint16_t k = 0X3412;
    ASSERT_EQ(std::memcmp(b.data(), &k, b.size()), 0);
    std::uint16_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 4660u);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::int32_t i = 305419896;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::int32_t k = 0X78563412;
    ASSERT_EQ(std::memcmp(b.data(), &k, b.size()), 0);
    std::int32_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 305419896);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint32_t i = 305419896;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::uint32_t k = 0X78563412;
    ASSERT_EQ(std::memcmp(b.data(), &k, b.size()), 0);
    std::uint32_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 305419896u);
  }
}

namespace {

enum class E1 : char {
  zero = 'z',
  val = 'a'
};

enum class E2 : std::uint16_t {
  zero = 0,
  val = 4660
};

enum class E3 : std::uint32_t {
  zero = 0,
  val = 305419896
};

} // namespace

TEST(packing, enumeration) {
  {
    E1 i = E1::val;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr char k = 0X61;
    ASSERT_EQ(std::memcmp(b.data(), &k, b.size()), 0);
    E1 j = E1::zero;
    unpack(j, b.data());
    ASSERT_EQ(j, E1::val);
  }
  {
    E2 i = E2::val;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::uint16_t k = 0X3412;
    ASSERT_EQ(std::memcmp(b.data(), &k, b.size()), 0);
    E2 j = E2::zero;
    unpack(j, b.data());
    ASSERT_EQ(j, E2::val);
  }
  {
    E3 i = E3::val;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::uint32_t k = 0X78563412;
    ASSERT_EQ(std::memcmp(b.data(), &k, b.size()), 0);
    E3 j = E3::zero;
    unpack(j, b.data());
    ASSERT_EQ(j, E3::val);
  }
}

TEST(packing, pack_alphanumeric) {
  constexpr auto size = 4;
  {
    std::string s = "";
    std::array<char, size> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(std::memcmp(b.data(), "    ", b.size()), 0);
  }
  {
    std::string s = "a";
    std::array<char, size> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(std::memcmp(b.data(), "a   ", b.size()), 0);
  }
  {
    std::string s = "ab";
    std::array<char, size> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(std::memcmp(b.data(), "ab  ", b.size()), 0);
  }
  {
    std::string s = "abc";
    std::array<char, size> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(std::memcmp(b.data(), "abc ", b.size()), 0);
  }
  {
    std::string s = "abcd";
    std::array<char, size> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(std::memcmp(b.data(), "abcd", b.size()), 0);
  }
  {
    std::string s = "  ab"; // Leading invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(std::memcmp(b.data(), "    ", b.size()), 0);
  }
  {
    std::string s = "ab__"; // Trailing invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(std::memcmp(b.data(), "ab  ", b.size()), 0);
  }
  {
    std::string s = "a  b"; // Embedded invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(std::memcmp(b.data(), "a   ", b.size()), 0);
  }
  {
    std::string s = "abcde"; // Too long.
    std::array<char, size> b = {};
    b.fill('*');
    pack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(std::memcmp(b.data(), "abcd", b.size()), 0);
  }
}

TEST(packing, unpack_alphanumeric) {
  constexpr auto size = 4;
  {
    std::string b = "    ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(s, ""s);
  }
  {
    std::string b = "a   ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(s, "a"s);
  }
  {
    std::string b = "ab  ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(s, "ab"s);
  }
  {
    std::string b = "abc ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(s, "abc"s);
  }
  {
    std::string b = "abcd";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(s, "abcd"s);
  }
  {
    std::string b = "  ab"; // Leading invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(s, ""s);
  }
  {
    std::string b = "ab__"; // Trailing invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(s, "ab"s);
  }
  {
    std::string b = "a  b"; // Embedded invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_alphanumeric(s, b.data(), size);
    ASSERT_EQ(s, "a"s);
  }
}

TEST(packing, pack_session) {
  constexpr auto size = 10;
  {
    std::string s = "";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "          ", b.size()), 0);
  }
  {
    std::string s = "a";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "         a", b.size()), 0);
  }
  {
    std::string s = "abcde";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "     abcde", b.size()), 0);
  }
  {
    std::string s = "abcdefghi";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), " abcdefghi", b.size()), 0);
  }
  {
    std::string s = "abcdefghij";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "abcdefghij", b.size()), 0);
  }
  {
    std::string s = "     abcde"; // Leading invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "          ", b.size()), 0);
  }
  {
    std::string s = "abcde_____"; // Trailing invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "     abcde", b.size()), 0);
  }
  {
    std::string s = "abc    def"; // Embedded invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "       abc", b.size()), 0);
  }
  {
    std::string s = "abcdefghijk"; // Too long.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "abcdefghij", b.size()), 0);
  }
}

TEST(packing, unpack_session) {
  constexpr auto size = 10;
  {
    std::string b = "          ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string b = "         a";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "a"s);
  }
  {
    std::string b = "     abcde";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcde"s);
  }
  {
    std::string b = " abcdefghi";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcdefghi"s);
  }
  {
    std::string b = "abcdefghij";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcdefghij"s);
  }
  {
    std::string b = "  ---abcde"; // Leading invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string b = "abcde_____"; // Trailing invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcde"s);
  }
  {
    std::string b = "abc    def"; // Embedded invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abc"s);
  }
}

TEST(packing, pack_sequence_number) {
  constexpr auto size = 20;
  {
    std::uint64_t i = 0;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "                   0", b.size()), 0);
  }
  {
    std::uint64_t i = 1;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "                   1", b.size()), 0);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint64_t i = 12345;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "               12345", b.size()), 0);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint64_t i = 1234567890;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "          1234567890", b.size()), 0);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint64_t i = 9999999999999999999uL;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), " 9999999999999999999", b.size()), 0);
  }
  {
    std::uint64_t i = std::numeric_limits<std::uint64_t>::max();
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    ASSERT_EQ(std::memcmp(b.data(), "18446744073709551615", b.size()), 0);
  }
}

TEST(packing, unpack_sequence_number) {
  constexpr auto size = 20;
  constexpr auto distinct_initial_value = 42;
  {
    std::string b = "                   0";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string b = "                   1";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1u);
  }
  {
    std::string b = "               12345";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 12345u);
  }
  {
    std::string b = "          1234567890";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1234567890u);
  }
  {
    std::string b = " 9999999999999999999";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 9999999999999999999uL);
  }
  {
    std::string b = "18446744073709551615";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, std::numeric_limits<std::uint64_t>::max());
  }
  {
    std::string b = "                    "; // No digits.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string b = "          123 567890"; // Non-digit.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 123u);
  }
  {
    std::string b = "18446744073709551616"; // Overflow.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string b = "18446744073709551617"; // Overflow.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1u);
  }
}
