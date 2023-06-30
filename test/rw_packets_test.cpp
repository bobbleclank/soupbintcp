#include "bc/soup/rw_packets.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#include <arpa/inet.h>

#include <gtest/gtest.h>

using namespace bc::soup;

namespace {

void assert_empty(const Buffer& b) {
  ASSERT_EQ(b.data(), nullptr);
  ASSERT_EQ(b.size(), 0u);
}

void assert_non_empty(const Buffer& b, std::size_t size) {
  ASSERT_NE(b.data(), nullptr);
  ASSERT_EQ(b.size(), size);
}

} // namespace

TEST(Buffer, default_constructor) {
  Buffer b;
  assert_empty(b);
}

TEST(Buffer, size_constructor) {
  {
    Buffer b(0);
    assert_non_empty(b, 0);
  }
  {
    Buffer b(5);
    assert_non_empty(b, 5);
  }
}

TEST(Buffer, move_constructor) {
  {
    Buffer b1;
    Buffer b2(std::move(b1));
    assert_empty(b1);
    assert_empty(b2);
  }
  {
    Buffer b1(5);
    Buffer b2(std::move(b1));
    assert_empty(b1);
    assert_non_empty(b2, 5);
  }
}

TEST(Buffer, move_assignment) {
  {
    Buffer b1;
    Buffer b2;
    b2 = std::move(b1);
    assert_empty(b1);
    assert_empty(b2);
  }
  {
    Buffer b1(5);
    Buffer b2;
    b2 = std::move(b1);
    assert_empty(b1);
    assert_non_empty(b2, 5);
  }
  {
    Buffer b1;
    Buffer b2(11);
    b2 = std::move(b1);
    assert_empty(b1);
    assert_empty(b2);
  }
  {
    Buffer b1(5);
    Buffer b2(11);
    b2 = std::move(b1);
    assert_empty(b1);
    assert_non_empty(b2, 5);
  }
}

TEST(Buffer, data) {
  Buffer b(5);
  std::memcpy(b.data(), "hello", 5);
  const Buffer& r = b;
  ASSERT_EQ(std::memcmp(r.data(), "hello", 5), 0);
}

TEST(Buffer, subscript_operator) {
  Buffer b(5);
  b[0] = std::byte('h');
  b[1] = std::byte('e');
  b[2] = std::byte('l');
  b[3] = std::byte('l');
  b[4] = std::byte('o');
  const Buffer& r = b;
  ASSERT_EQ(r[0], std::byte('h'));
  ASSERT_EQ(r[1], std::byte('e'));
  ASSERT_EQ(r[2], std::byte('l'));
  ASSERT_EQ(r[3], std::byte('l'));
  ASSERT_EQ(r[4], std::byte('o'));
}

namespace {

void write_header(Read_packet& p, std::uint16_t packet_size, char packet_type) {
  auto* ptr = static_cast<unsigned char*>(p.header_data());
  packet_size = htons(packet_size);
  std::memcpy(ptr, &packet_size, sizeof(packet_size));
  std::memcpy(ptr + sizeof(packet_size), &packet_type, sizeof(packet_type));
}

void write_payload(Read_packet& p, char packet_type, std::uint16_t payload_size,
                   const void* payload_data) {
  write_header(p, 1u + payload_size, packet_type);
  (void)p.resize_payload();
  std::memcpy(p.payload_data(), payload_data, p.payload_size());
}

void assert_empty(const Read_packet& p, std::uint16_t packet_size,
                  char packet_type) {
  ASSERT_EQ(p.packet_size(), packet_size);
  ASSERT_EQ(p.packet_type(), packet_type);
  ASSERT_EQ(p.payload_data(), nullptr);
  ASSERT_EQ(p.payload_size(), 0u);
}

void assert_empty(const Read_packet& p) { assert_empty(p, 0, '\0'); }

void assert_non_empty(const Read_packet& p, char packet_type,
                      std::uint16_t payload_size) {
  ASSERT_EQ(p.packet_size(), 1u + payload_size);
  ASSERT_EQ(p.packet_type(), packet_type);
  ASSERT_NE(p.payload_data(), nullptr);
  ASSERT_EQ(p.payload_size(), payload_size);
}

void assert_non_empty(const Read_packet& p, char packet_type,
                      std::uint16_t payload_size, const void* payload_data) {
  assert_non_empty(p, packet_type, payload_size);
  ASSERT_EQ(std::memcmp(p.payload_data(), payload_data, payload_size), 0);
}

} // namespace

TEST(Read_packet, default_constructor) {
  Read_packet p;
  ASSERT_NE(p.header_data(), nullptr);
  constexpr auto header_size = p.header_size();
  ASSERT_EQ(header_size, 3u);
  assert_empty(p);
}

TEST(Read_packet, header) {
  Read_packet p;
  write_header(p, 1 + 5, 'a');
  assert_empty(p, 1 + 5, 'a');
}

TEST(Read_packet, move_constructor) {
  {
    Read_packet p1;
    Read_packet p2(std::move(p1));
    assert_empty(p1);
    assert_empty(p2);
  }
  {
    Read_packet p1;
    write_payload(p1, 'a', 5, "hello");
    Read_packet p2(std::move(p1));
    assert_empty(p1);
    assert_non_empty(p2, 'a', 5, "hello");
  }
}

TEST(Read_packet, move_assignment) {
  {
    Read_packet p1;
    Read_packet p2;
    p2 = std::move(p1);
    assert_empty(p1);
    assert_empty(p2);
  }
  {
    Read_packet p1;
    write_payload(p1, 'a', 5, "hello");
    Read_packet p2;
    p2 = std::move(p1);
    assert_empty(p1);
    assert_non_empty(p2, 'a', 5, "hello");
  }
  {
    Read_packet p1;
    Read_packet p2;
    write_payload(p2, 'b', 11, "HELLO WORLD");
    p2 = std::move(p1);
    assert_empty(p1);
    assert_empty(p2);
  }
  {
    Read_packet p1;
    write_payload(p1, 'a', 5, "hello");
    Read_packet p2;
    write_payload(p2, 'b', 11, "HELLO WORLD");
    p2 = std::move(p1);
    assert_empty(p1);
    assert_non_empty(p2, 'a', 5, "hello");
  }
}

TEST(Read_packet, resize_payload) {
  {
    Read_packet p;
    ASSERT_EQ(p.resize_payload(), Read_packet::Resize_result::bad_packet);
    assert_empty(p);
  }
  {
    Read_packet p;
    write_header(p, 1 + 0, 'a');
    ASSERT_EQ(p.resize_payload(), Read_packet::Resize_result::empty_payload);
    assert_empty(p, 1, 'a');
  }
  {
    Read_packet p;
    write_header(p, 1 + 1, 'a');
    ASSERT_EQ(p.resize_payload(), Read_packet::Resize_result::resized);
    assert_non_empty(p, 'a', 1);
  }
  {
    Read_packet p;
    write_payload(p, 'a', 5, "hello");
    write_header(p, 0 + 0, 'b');
    ASSERT_EQ(p.resize_payload(), Read_packet::Resize_result::bad_packet);
    assert_empty(p, 0, 'b');
  }
  {
    Read_packet p;
    write_payload(p, 'a', 5, "hello");
    write_header(p, 1 + 0, 'b');
    ASSERT_EQ(p.resize_payload(), Read_packet::Resize_result::empty_payload);
    assert_empty(p, 1, 'b');
  }
  {
    Read_packet p;
    write_payload(p, 'a', 5, "hello");
    write_header(p, 1 + 1, 'b');
    ASSERT_EQ(p.resize_payload(), Read_packet::Resize_result::resized);
    assert_non_empty(p, 'b', 1);
  }
}

TEST(Read_packet, payload) {
  Read_packet p;
  write_payload(p, 'a', 5, "hello");
  assert_non_empty(p, 'a', 5, "hello");
}

namespace {

void assert_empty(const Write_packet& p) {
  ASSERT_EQ(p.payload_capacity(), static_cast<std::size_t>(-3));
  ASSERT_EQ(p.data(), nullptr);
}

void assert_non_empty(const Write_packet& p, char packet_type,
                      std::uint16_t payload_capacity,
                      std::uint16_t payload_size) {
  ASSERT_EQ(p.payload_capacity(), payload_capacity);
  ASSERT_NE(p.data(), nullptr);
  ASSERT_EQ(p.packet_size(), 1u + payload_size);
  ASSERT_EQ(p.packet_type(), packet_type);
  ASSERT_EQ(p.payload_size(), payload_size);
  ASSERT_EQ(p.size(), 3u + payload_size);
}

void assert_non_empty(const Write_packet& p, char packet_type,
                      std::uint16_t payload_capacity,
                      std::uint16_t payload_size, const void* payload_data) {
  assert_non_empty(p, packet_type, payload_capacity, payload_size);
  ASSERT_EQ(std::memcmp(p.payload_data(), payload_data, payload_size), 0);
}

} // namespace

TEST(Write_packet, default_constructor) {
  Write_packet p;
  assert_empty(p);
}

TEST(Write_packet, empty_payload_constructor) {
  Write_packet p('a');
  assert_non_empty(p, 'a', 0, 0);
}

TEST(Write_packet, allocate_payload_constructor) {
  Write_packet p('a', 5);
  assert_non_empty(p, 'a', 5, 5);
}

TEST(Write_packet, write_payload_constructor) {
  Write_packet p('a', "hello", 5);
  assert_non_empty(p, 'a', 5, 5, "hello");
}

TEST(Write_packet, move_constructor) {
  {
    Write_packet p1;
    Write_packet p2(std::move(p1));
    assert_empty(p1);
    assert_empty(p2);
  }
  {
    Write_packet p1('a', "hello", 5);
    Write_packet p2(std::move(p1));
    assert_empty(p1);
    assert_non_empty(p2, 'a', 5, 5, "hello");
  }
}

TEST(Write_packet, move_assignment) {
  {
    Write_packet p1;
    Write_packet p2;
    p2 = std::move(p1);
    assert_empty(p1);
    assert_empty(p2);
  }
  {
    Write_packet p1('a', "hello", 5);
    Write_packet p2;
    p2 = std::move(p1);
    assert_empty(p1);
    assert_non_empty(p2, 'a', 5, 5, "hello");
  }
  {
    Write_packet p1;
    Write_packet p2('b', "HELLO WORLD", 11);
    p2 = std::move(p1);
    assert_empty(p1);
    assert_empty(p2);
  }
  {
    Write_packet p1('a', "hello", 5);
    Write_packet p2('b', "HELLO WORLD", 11);
    p2 = std::move(p1);
    assert_empty(p1);
    assert_non_empty(p2, 'a', 5, 5, "hello");
  }
}

TEST(Write_packet, resize_payload) {
  {
    Write_packet p('a');
    ASSERT_EQ(p.resize_payload(1), Write_packet::Resize_result::no_capacity);
    assert_non_empty(p, 'a', 0, 0);
    ASSERT_EQ(p.resize_payload(0), Write_packet::Resize_result::resized);
    assert_non_empty(p, 'a', 0, 0);
  }
  {
    Write_packet p('a', 1);
    ASSERT_EQ(p.resize_payload(2), Write_packet::Resize_result::no_capacity);
    assert_non_empty(p, 'a', 1, 1);
    ASSERT_EQ(p.resize_payload(1), Write_packet::Resize_result::resized);
    assert_non_empty(p, 'a', 1, 1);
    ASSERT_EQ(p.resize_payload(0), Write_packet::Resize_result::resized);
    assert_non_empty(p, 'a', 1, 0);
  }
  {
    Write_packet p('a', "hello world", 11);
    ASSERT_EQ(p.resize_payload(5), Write_packet::Resize_result::resized);
    assert_non_empty(p, 'a', 11, 5, "hello");
    ASSERT_EQ(p.resize_payload(0), Write_packet::Resize_result::resized);
    assert_non_empty(p, 'a', 11, 0);
    ASSERT_EQ(p.resize_payload(11), Write_packet::Resize_result::resized);
    assert_non_empty(p, 'a', 11, 11, "hello world");
  }
}

TEST(Write_packet, payload) {
  Write_packet p('a', 5);
  std::memcpy(p.payload_data(), "hello", 5);
  assert_non_empty(p, 'a', 5, 5, "hello");
}
