#include "bc/soup/client/message.h"
#include "bc/soup/server/message.h"

#include "bc/soup/constants.h"
#include "bc/soup/rw_packets.h"

#include <array>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

using namespace bc::soup;

namespace {

template <typename Message>
consteval char packet_type() {
  if (std::is_same_v<Message, client::Message>)
    return 'U';
  if (std::is_same_v<Message, server::Message>)
    return 'S';
  std::unreachable();
}

void assert_empty(const Write_packet& p, char) {
  ASSERT_EQ(p.payload_capacity(), 0u);
  ASSERT_EQ(p.data(), nullptr);
}

void assert_non_empty(const Write_packet& p, char packet_type,
                      std::size_t payload_capacity, std::size_t payload_size) {
  ASSERT_EQ(p.payload_capacity(), payload_capacity);
  ASSERT_NE(p.data(), nullptr);
  ASSERT_EQ(p.packet_size(), 1 + payload_size);
  ASSERT_EQ(p.packet_type(), packet_type);
  ASSERT_EQ(p.payload_size(), payload_size);
  ASSERT_EQ(p.size(), 3 + payload_size);
}

void assert_non_empty(const Write_packet& p, char packet_type,
                      std::size_t payload_capacity, std::size_t payload_size,
                      const void* payload_data) {
  assert_non_empty(p, packet_type, payload_capacity, payload_size);
  ASSERT_EQ(std::memcmp(p.payload_data(), payload_data, payload_size), 0);
}

template <typename Message>
void assert_empty(Message& m) {
  ASSERT_EQ(m.payload_capacity(), 0u);
  Write_packet p = m.release_packet();
  assert_empty(p, packet_type<Message>());
}

template <typename Message>
void assert_non_empty(Message& m, std::size_t payload_capacity,
                      std::size_t payload_size) {
  ASSERT_EQ(m.payload_capacity(), payload_capacity);
  ASSERT_EQ(m.payload_size(), payload_size);
  Write_packet p = m.release_packet();
  assert_non_empty(p, packet_type<Message>(), payload_capacity, payload_size);
}

template <typename Message>
void assert_non_empty(Message& m, std::size_t payload_capacity,
                      std::size_t payload_size, const void* payload_data) {
  ASSERT_EQ(m.payload_capacity(), payload_capacity);
  ASSERT_EQ(m.payload_size(), payload_size);
  ASSERT_EQ(std::memcmp(m.payload_data(), payload_data, payload_size), 0);
  Write_packet p = m.release_packet();
  assert_non_empty(p, packet_type<Message>(), payload_capacity, payload_size,
                   payload_data);
}

} // namespace

template <typename T>
class Message : public ::testing::Test {};

using Types = ::testing::Types<client::Message, server::Message>;
TYPED_TEST_SUITE(Message, Types, );

TYPED_TEST(Message, default_constructor) {
  TypeParam m;
  assert_empty(m);
}

TYPED_TEST(Message, allocate_payload_constructor) {
  {
    TypeParam m(0);
    assert_non_empty(m, 0, 0);
  }
  {
    constexpr auto size = 1;
    TypeParam m(size);
    assert_non_empty(m, size, size);
  }
  {
    constexpr auto size = 5;
    std::array<unsigned char, size> arr = {};

    TypeParam m(size);
    // Payload zeroed
    assert_non_empty(m, size, size, arr.data());
  }
  {
    TypeParam m(max_payload_size);
    assert_non_empty(m, max_payload_size, max_payload_size);
  }
}

TYPED_TEST(Message, write_payload_constructor) {
  {
    std::string_view sv = "";
    const auto* data = sv.data();
    ASSERT_NE(data, nullptr);

    TypeParam m(data, 0);
    assert_non_empty(m, 0, 0);
  }
  {
    std::string_view sv = "h";
    const auto* data = sv.data();
    auto size = sv.size();
    ASSERT_EQ(size, 1u);

    TypeParam m(data, size);
    assert_non_empty(m, size, size, data);
  }
  {
    std::string_view sv = "hello";
    const auto* data = sv.data();
    auto size = sv.size();
    ASSERT_EQ(size, 5u);

    TypeParam m(data, size);
    assert_non_empty(m, size, size, data);
  }
  {
    std::string str(max_payload_size, 'o');
    const auto* data = str.data();
    ASSERT_EQ(str.size(), max_payload_size);

    TypeParam m(data, max_payload_size);
    assert_non_empty(m, max_payload_size, max_payload_size, data);
  }
}

TYPED_TEST(Message, release_packet) {
  {
    TypeParam m;
    Write_packet p = m.release_packet();
    assert_empty(m);
    assert_empty(p, packet_type<TypeParam>());
  }
  {
    std::string_view sv = "hello";
    const auto* data = sv.data();
    auto size = sv.size();
    ASSERT_EQ(size, 5u);

    TypeParam m(data, size);
    Write_packet p = m.release_packet();
    assert_empty(m);
    assert_non_empty(p, packet_type<TypeParam>(), size, size, data);
  }
}

TYPED_TEST(Message, resize_payload) {
  {
    TypeParam m(0);

    constexpr auto new_size = 1;
    typename TypeParam::Resize_result result = m.resize_payload(new_size);
    ASSERT_EQ(result, TypeParam::Resize_result::no_capacity);
    ASSERT_EQ(m.payload_capacity(), 0u);
    ASSERT_EQ(m.payload_size(), 0u);

    result = m.resize_payload(0);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    assert_non_empty(m, 0, 0);
  }
  {
    TypeParam m(1);

    constexpr auto new_size = 2;
    typename TypeParam::Resize_result result = m.resize_payload(new_size);
    ASSERT_EQ(result, TypeParam::Resize_result::no_capacity);
    ASSERT_EQ(m.payload_capacity(), 1u);
    ASSERT_EQ(m.payload_size(), 1u);

    result = m.resize_payload(1);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    ASSERT_EQ(m.payload_capacity(), 1u);
    ASSERT_EQ(m.payload_size(), 1u);

    result = m.resize_payload(0);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    assert_non_empty(m, 1, 0);
  }
  {
    std::string_view sv = "hello world";
    const auto* data = sv.data();
    auto size = sv.size();
    ASSERT_EQ(size, 11u);

    TypeParam m(data, size);

    typename TypeParam::Resize_result result = m.resize_payload(size + 1);
    ASSERT_EQ(result, TypeParam::Resize_result::no_capacity);
    ASSERT_EQ(m.payload_capacity(), size);
    ASSERT_EQ(m.payload_size(), size);
    ASSERT_EQ(std::memcmp(m.payload_data(), data, size), 0);

    constexpr auto new_size = 5;
    result = m.resize_payload(new_size);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    ASSERT_EQ(m.payload_capacity(), 11u);
    ASSERT_EQ(m.payload_size(), 5u);
    ASSERT_EQ(std::memcmp(m.payload_data(), data, new_size), 0);

    result = m.resize_payload(0);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    ASSERT_EQ(m.payload_capacity(), 11u);
    ASSERT_EQ(m.payload_size(), 0u);

    result = m.resize_payload(size);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    // Spare capacity kept
    assert_non_empty(m, size, size, data);
  }
  {
    TypeParam m(max_payload_size);

    typename TypeParam::Resize_result result =
        m.resize_payload(max_payload_size + 1);
    ASSERT_EQ(result, TypeParam::Resize_result::no_capacity);
    ASSERT_EQ(m.payload_capacity(), max_payload_size);
    ASSERT_EQ(m.payload_size(), max_payload_size);

    result = m.resize_payload(0);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    ASSERT_EQ(m.payload_capacity(), max_payload_size);
    ASSERT_EQ(m.payload_size(), 0u);

    result = m.resize_payload(max_payload_size);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    assert_non_empty(m, max_payload_size, max_payload_size);
  }
}

TYPED_TEST(Message, payload) {
  std::string_view sv = "hello";
  const auto* data = sv.data();
  auto size = sv.size();
  ASSERT_EQ(size, 5u);

  TypeParam m(size);
  std::memcpy(m.payload_data(), data, size);
  assert_non_empty(m, size, size, data);
}
