#include "bc/soup/packing.h"

#include "bc/soup/constants.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <limits>
#include <span>

namespace bc::soup {
namespace {

// Username and password are alphanumeric and padded on the right with spaces.

template <std::size_t length>
void pack_alphanumeric_right_padded(std::string_view str, void* data) {
  const std::span<char, length> s(static_cast<char*>(data), length);
  if (str.size() > s.size())
    str.remove_suffix(str.size() - s.size());
  const auto iter =
      std::ranges::find_if_not(str, [](auto c) { return std::isalnum(c); });
  str.remove_suffix(str.end() - iter);
  const auto pad = s.last(s.size() - str.size());
  std::ranges::copy(str, s.begin());
  std::ranges::fill(pad, ' ');
}

template <std::size_t length>
void unpack_alphanumeric_right_padded(std::string& str, const void* data) {
  const std::span<const char, length> s(static_cast<const char*>(data), length);
  const auto iter =
      std::ranges::find_if_not(s, [](auto c) { return std::isalnum(c); });
  const auto sub = s.first(iter - s.begin());
  str.insert(str.begin(), sub.begin(), sub.end());
}

} // namespace

void pack_username(std::string_view str, void* data) {
  pack_alphanumeric_right_padded<username_length>(str, data);
}

void unpack_username(std::string& str, const void* data) {
  unpack_alphanumeric_right_padded<username_length>(str, data);
}

void pack_password(std::string_view str, void* data) {
  pack_alphanumeric_right_padded<password_length>(str, data);
}

void unpack_password(std::string& str, const void* data) {
  unpack_alphanumeric_right_padded<password_length>(str, data);
}

namespace {

// Session is alphanumeric and padded on the left with spaces.

template <std::size_t length>
void pack_alphanumeric_left_padded(std::string_view str, void* data) {
  const std::span<char, length> s(static_cast<char*>(data), length);
  if (str.size() > s.size())
    str.remove_suffix(str.size() - s.size());
  const auto iter = std::ranges::find_if_not(
      str.rbegin(), str.rend(), [](auto c) { return std::isalnum(c); });
  str.remove_prefix(str.rend() - iter);
  const auto pad = s.first(s.size() - str.size());
  const auto sub = s.last(str.size());
  std::ranges::fill(pad, ' ');
  std::ranges::copy(str, sub.begin());
}

template <std::size_t length>
void unpack_alphanumeric_left_padded(std::string& str, const void* data) {
  const std::span<const char, length> s(static_cast<const char*>(data), length);
  const auto iter = std::ranges::find_if_not(
      s.rbegin(), s.rend(), [](auto c) { return std::isalnum(c); });
  const auto sub = s.last(iter - s.rbegin());
  str.insert(str.begin(), sub.begin(), sub.end());
}

} // namespace

void pack_session(std::string_view str, void* data) {
  pack_alphanumeric_left_padded<session_length>(str, data);
}

void unpack_session(std::string& str, const void* data) {
  unpack_alphanumeric_left_padded<session_length>(str, data);
}

namespace {

// Sequence number is numeric and padded on the left with spaces.

template <typename Integral, std::size_t length>
void pack_numeric(Integral i, void* data) {
  static_assert(std::numeric_limits<Integral>::digits10 + 1 == length);
  constexpr auto base = 10;
  const std::span<char, length> s(static_cast<char*>(data), length);
  auto iter = s.end();
  // NOLINTNEXTLINE(*-avoid-do-while): Clear statement of a solution
  do {
    const char c = '0' + (i % base);
    i /= base;
    --iter;
    *iter = c;
  } while (i != 0);
  const auto pad = s.first(iter - s.begin());
  std::memset(pad.data(), ' ', pad.size());
}

template <typename Integral, std::size_t length>
void unpack_numeric(Integral& i, const void* data) {
  static_assert(std::numeric_limits<Integral>::digits10 + 1 == length);
  constexpr auto base = 10;
  const std::span<const char, length> s(static_cast<const char*>(data), length);
  const auto iter = std::ranges::find_if_not(
      s.rbegin(), s.rend(), [](auto c) { return std::isdigit(c); });
  const auto sub = s.last(iter - s.rbegin());
  i = 0;
  for (const char c : sub) {
    i *= base;
    i += c - '0';
  }
}

} // namespace

void pack_sequence_number(std::uint64_t i, void* data) {
  pack_numeric<std::uint64_t, sequence_number_length>(i, data);
}

void unpack_sequence_number(std::uint64_t& i, const void* data) {
  unpack_numeric<std::uint64_t, sequence_number_length>(i, data);
}

} // namespace bc::soup
