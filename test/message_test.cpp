#include "bc/soup/client/message.h"
#include "bc/soup/server/message.h"

#include "bc/soup/rw_packets.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <gtest/gtest.h>

using namespace bc::soup;

namespace {

template <typename Message> struct Packet_type;

template <> struct Packet_type<client::Message> {
  static constexpr char value = 'U';
};

template <> struct Packet_type<server::Message> {
  static constexpr char value = 'S';
};

void assert_empty(const Write_packet& p, char) {
  ASSERT_EQ(p.payload_capacity(), static_cast<std::size_t>(-3));
  ASSERT_EQ(p.data(), nullptr);
}

void assert_non_empty(const Write_packet& p, char packet_type,
                      std::uint16_t payload_capacity,
                      std::uint16_t payload_size) {
  ASSERT_EQ(p.payload_capacity(), payload_capacity);
  ASSERT_NE(p.data(), nullptr);
  ASSERT_EQ(p.packet_size(), 1 + payload_size);
  ASSERT_EQ(p.packet_type(), packet_type);
  ASSERT_EQ(p.payload_size(), payload_size);
  ASSERT_EQ(p.size(), 3 + payload_size);
}

void assert_non_empty(const Write_packet& p, char packet_type,
                      std::uint16_t payload_capacity,
                      std::uint16_t payload_size, const void* payload_data) {
  assert_non_empty(p, packet_type, payload_capacity, payload_size);
  ASSERT_EQ(std::memcmp(p.payload_data(), payload_data, payload_size), 0);
}

template <typename Message> void assert_empty(Message& m) {
  ASSERT_EQ(m.payload_capacity(), static_cast<std::size_t>(-3));
  Write_packet p = m.release_packet();
  assert_empty(p, Packet_type<Message>::value);
}

template <typename Message>
void assert_non_empty(Message& m, std::uint16_t payload_capacity,
                      std::uint16_t payload_size) {
  ASSERT_EQ(m.payload_capacity(), payload_capacity);
  ASSERT_EQ(m.payload_size(), payload_size);
  Write_packet p = m.release_packet();
  assert_non_empty(p, Packet_type<Message>::value, payload_capacity,
                   payload_size);
}
template <typename Message>
void assert_non_empty(Message& m, std::uint16_t payload_capacity,
                      std::uint16_t payload_size, const void* payload_data) {
  ASSERT_EQ(m.payload_capacity(), payload_capacity);
  ASSERT_EQ(m.payload_size(), payload_size);
  ASSERT_EQ(std::memcmp(m.payload_data(), payload_data, payload_size), 0);
  Write_packet p = m.release_packet();
  assert_non_empty(p, Packet_type<Message>::value, payload_capacity,
                   payload_size, payload_data);
}

} // namespace

template <typename T> class Message : public ::testing::Test {};

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
    TypeParam m(5);
    assert_non_empty(m, 5, 5);
  }
}

TYPED_TEST(Message, write_payload_constructor) {
  TypeParam m("hello", 5);
  assert_non_empty(m, 5, 5, "hello");
}

TYPED_TEST(Message, release_packet) {
  {
    TypeParam m;
    Write_packet p = m.release_packet();
    assert_empty(m);
    assert_empty(p, Packet_type<TypeParam>::value);
  }
  {
    TypeParam m("hello", 5);
    Write_packet p = m.release_packet();
    assert_empty(m);
    assert_non_empty(p, Packet_type<TypeParam>::value, 5, 5, "hello");
  }
  {
    TypeParam m("hello", 5);
    Write_packet p = m.release_packet();
    Write_packet q = m.release_packet();
    (void)p;
    assert_empty(q, Packet_type<TypeParam>::value);
  }
}

TYPED_TEST(Message, resize_payload) {
  {
    TypeParam m(0);

    typename TypeParam::Resize_result result = m.resize_payload(1);
    ASSERT_EQ(result, TypeParam::Resize_result::no_capacity);
    ASSERT_EQ(m.payload_capacity(), 0);
    ASSERT_EQ(m.payload_size(), 0);

    result = m.resize_payload(0);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    assert_non_empty(m, 0, 0);
  }
  {
    TypeParam m(1);

    typename TypeParam::Resize_result result = m.resize_payload(2);
    ASSERT_EQ(result, TypeParam::Resize_result::no_capacity);
    ASSERT_EQ(m.payload_capacity(), 1);
    ASSERT_EQ(m.payload_size(), 1);

    result = m.resize_payload(1);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    ASSERT_EQ(m.payload_capacity(), 1);
    ASSERT_EQ(m.payload_size(), 1);

    result = m.resize_payload(0);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    assert_non_empty(m, 1, 0);
  }
  {
    TypeParam m("hello world", 11);

    typename TypeParam::Resize_result result = m.resize_payload(5);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    ASSERT_EQ(m.payload_capacity(), 11);
    ASSERT_EQ(m.payload_size(), 5);
    ASSERT_EQ(std::memcmp(m.payload_data(), "hello", 5), 0);

    result = m.resize_payload(0);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    ASSERT_EQ(m.payload_capacity(), 11);
    ASSERT_EQ(m.payload_size(), 0);

    result = m.resize_payload(11);
    ASSERT_EQ(result, TypeParam::Resize_result::resized);
    assert_non_empty(m, 11, 11, "hello world");
  }
}

TYPED_TEST(Message, payload) {
  TypeParam m(5);
  std::memcpy(m.payload_data(), "hello", 5);
  assert_non_empty(m, 5, 5, "hello");
}
