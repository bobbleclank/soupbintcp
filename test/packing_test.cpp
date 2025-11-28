#include "bc/soup/packing.h"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

using namespace bc::soup;

using namespace std::string_literals;

TEST(packing, integral) {
  {
    char i = 'a';
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr char e = 0X61;
    ASSERT_EQ(std::memcmp(b.data(), &e, b.size()), 0);
    char j = '\0';
    unpack(j, b.data());
    ASSERT_EQ(j, 'a');
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::int16_t i = 4660;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::int16_t e = 0X3412;
    ASSERT_EQ(std::memcmp(b.data(), &e, b.size()), 0);
    std::int16_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 4660);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint16_t i = 4660;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::uint16_t e = 0X3412;
    ASSERT_EQ(std::memcmp(b.data(), &e, b.size()), 0);
    std::uint16_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 4660u);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::int32_t i = 305419896;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::int32_t e = 0X78563412;
    ASSERT_EQ(std::memcmp(b.data(), &e, b.size()), 0);
    std::int32_t j = 0;
    unpack(j, b.data());
    ASSERT_EQ(j, 305419896);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint32_t i = 305419896;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::uint32_t e = 0X78563412;
    ASSERT_EQ(std::memcmp(b.data(), &e, b.size()), 0);
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
    constexpr char e = 0X61;
    ASSERT_EQ(std::memcmp(b.data(), &e, b.size()), 0);
    E1 j = E1::zero;
    unpack(j, b.data());
    ASSERT_EQ(j, E1::val);
  }
  {
    E2 i = E2::val;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::uint16_t e = 0X3412;
    ASSERT_EQ(std::memcmp(b.data(), &e, b.size()), 0);
    E2 j = E2::zero;
    unpack(j, b.data());
    ASSERT_EQ(j, E2::val);
  }
  {
    E3 i = E3::val;
    std::array<unsigned char, sizeof(i)> b = {};
    pack(i, b.data());
    constexpr std::uint32_t e = 0X78563412;
    ASSERT_EQ(std::memcmp(b.data(), &e, b.size()), 0);
    E3 j = E3::zero;
    unpack(j, b.data());
    ASSERT_EQ(j, E3::val);
  }
}

TEST(packing, pack_password) {
  constexpr auto size = 10;
  {
    std::string_view s = "";
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "          ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "a";
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "a         ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcde";
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abcde     ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghi";
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abcdefghi ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghij";
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abcdefghij";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "     abcde"; // Leading invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "          ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "_____abcde"; // Leading invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "          ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcde     "; // Trailing invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abcde     ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcde_____"; // Trailing invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abcde     ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abc    def"; // Embedded invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abc       ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abc____def"; // Embedded invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abc       ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghijkl"; // Too long.
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abcdefghij";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghij  "; // Too long (trailing invalid).
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abcdefghij";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghij__"; // Too long (trailing invalid).
    std::array<char, size> b = {};
    b.fill('*');
    pack_password(s, b.data());
    std::string_view e = "abcdefghij";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
}

TEST(packing, unpack_password) {
  constexpr auto size = 10;
  {
    std::string_view b = "          ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string_view b = "a         ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, "a"s);
  }
  {
    std::string_view b = "abcde     ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, "abcde"s);
  }
  {
    std::string_view b = "abcdefghi ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, "abcdefghi"s);
  }
  {
    std::string_view b = "abcdefghij";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, "abcdefghij"s);
  }
  {
    std::string_view b = "     abcde"; // Leading invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string_view b = "_____abcde"; // Leading invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string_view b = "abcde_____"; // Trailing invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, "abcde"s);
  }
  {
    std::string_view b = "abc    def"; // Embedded invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, "abc"s);
  }
  {
    std::string_view b = "abc____def"; // Embedded invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_password(s, b.data());
    ASSERT_EQ(s, "abc"s);
  }
}

TEST(packing, pack_session) {
  constexpr auto size = 10;
  {
    std::string_view s = "";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "          ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "a";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "         a";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcde";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "     abcde";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghi";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = " abcdefghi";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghij";
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "abcdefghij";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "     abcde"; // Leading invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "          ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "_____abcde"; // Leading invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "          ";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcde     "; // Trailing invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "     abcde";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcde_____"; // Trailing invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "     abcde";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abc    def"; // Embedded invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "       abc";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abc____def"; // Embedded invalid character.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "       abc";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghijkl"; // Too long.
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "abcdefghij";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghij  "; // Too long (trailing invalid).
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "abcdefghij";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::string_view s = "abcdefghij__"; // Too long (trailing invalid).
    std::array<char, size> b = {};
    b.fill('*');
    pack_session(s, b.data());
    std::string_view e = "abcdefghij";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
}

TEST(packing, unpack_session) {
  constexpr auto size = 10;
  {
    std::string_view b = "          ";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string_view b = "         a";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "a"s);
  }
  {
    std::string_view b = "     abcde";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcde"s);
  }
  {
    std::string_view b = " abcdefghi";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcdefghi"s);
  }
  {
    std::string_view b = "abcdefghij";
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcdefghij"s);
  }
  {
    std::string_view b = "_____abcde"; // Leading invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, ""s);
  }
  {
    std::string_view b = "abcde     "; // Trailing invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcde"s);
  }
  {
    std::string_view b = "abcde_____"; // Trailing invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abcde"s);
  }
  {
    std::string_view b = "abc    def"; // Embedded invalid character.
    ASSERT_EQ(b.size(), size);
    std::string s;
    unpack_session(s, b.data());
    ASSERT_EQ(s, "abc"s);
  }
  {
    std::string_view b = "abc____def"; // Embedded invalid character.
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
    std::string_view e = "                   0";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::uint64_t i = 1;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    std::string_view e = "                   1";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint64_t i = 12345;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    std::string_view e = "               12345";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint64_t i = 1234567890;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    std::string_view e = "          1234567890";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint64_t i = 123456789012345uL;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    std::string_view e = "     123456789012345";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    // NOLINTNEXTLINE(*-avoid-magic-numbers): Test value
    std::uint64_t i = 9999999999999999999uL;
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    std::string_view e = " 9999999999999999999";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
  {
    std::uint64_t i = std::numeric_limits<std::uint64_t>::max();
    std::array<char, size> b = {};
    b.fill('*');
    pack_sequence_number(i, b.data());
    std::string_view e = "18446744073709551615";
    ASSERT_EQ(e.size(), size);
    ASSERT_EQ(std::memcmp(b.data(), e.data(), b.size()), 0);
  }
}

TEST(packing, unpack_sequence_number) {
  constexpr auto size = 20;
  constexpr auto distinct_initial_value = 42;
  {
    std::string_view b = "                   0";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string_view b = "                   1";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1u);
  }
  {
    std::string_view b = "               12345";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 12345u);
  }
  {
    std::string_view b = "          1234567890";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1234567890u);
  }
  {
    std::string_view b = "     123456789012345";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 123456789012345uL);
  }
  {
    std::string_view b = " 9999999999999999999";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 9999999999999999999uL);
  }
  {
    std::string_view b = "18446744073709551615";
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, std::numeric_limits<std::uint64_t>::max());
  }
  {
    std::string_view b = "                    "; // No digits.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string_view b = "__________1234567890"; // Leading invalid character.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string_view b = "1234567890          "; // Trailing invalid character.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1234567890u);
  }
  {
    std::string_view b = "1234567890__________"; // Trailing invalid character.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1234567890u);
  }
  {
    std::string_view b = "12345          67890"; // Embedded invalid character.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 12345u);
  }
  {
    std::string_view b = "12345__________67890"; // Embedded invalid character.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 12345u);
  }
  {
    std::string_view b = "18446744073709551616"; // Overflow.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 0u);
  }
  {
    std::string_view b = "18446744073709551617"; // Overflow.
    ASSERT_EQ(b.size(), size);
    std::uint64_t i = distinct_initial_value;
    unpack_sequence_number(i, b.data());
    ASSERT_EQ(i, 1u);
  }
}
