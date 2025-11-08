#include "bc/soup/file_store.h"

#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <unistd.h>

#include <gtest/gtest.h>

using namespace bc::soup;

namespace {

const std::string filename = "test_store";
const std::string filename_2 = "test_store_2";

auto add(File_store& s, std::string_view sv) {
  return s.add(sv.data(), sv.size());
}

void assert_messages(const std::vector<Message>& v) {
  ASSERT_EQ(v.size(), 9u);
  auto i = v.begin();
  ASSERT_EQ(i->size(), 3u);
  ASSERT_EQ(std::memcmp(i->data(), "The", i->size()), 0);
  ++i;
  ASSERT_EQ(i->size(), 5u);
  ASSERT_EQ(std::memcmp(i->data(), "quick", i->size()), 0);
  ++i;
  ASSERT_EQ(i->size(), 5u);
  ASSERT_EQ(std::memcmp(i->data(), "brown", i->size()), 0);
  ++i;
  ASSERT_EQ(i->size(), 3u);
  ASSERT_EQ(std::memcmp(i->data(), "fox", i->size()), 0);
  ++i;
  ASSERT_EQ(i->size(), 5u);
  ASSERT_EQ(std::memcmp(i->data(), "jumps", i->size()), 0);
  ++i;
  ASSERT_EQ(i->size(), 4u);
  ASSERT_EQ(std::memcmp(i->data(), "over", i->size()), 0);
  ++i;
  ASSERT_EQ(i->size(), 3u);
  ASSERT_EQ(std::memcmp(i->data(), "the", i->size()), 0);
  ++i;
  ASSERT_EQ(i->size(), 4u);
  ASSERT_EQ(std::memcmp(i->data(), "lazy", i->size()), 0);
  ++i;
  ASSERT_EQ(i->size(), 3u);
  ASSERT_EQ(std::memcmp(i->data(), "dog", i->size()), 0);
  ++i;
  ASSERT_EQ(i, v.end());
}

} // namespace

TEST(File_store, add_to_empty_file) {
  unlink(filename.c_str());

  File_store s(filename);
  auto ec = s.open();
  ASSERT_FALSE(ec);
  ASSERT_EQ(s.next_sequence_number(), 1u);

  ec = add(s, "The");
  ASSERT_FALSE(ec);
  ec = add(s, "quick");
  ASSERT_FALSE(ec);
  ec = add(s, "brown");
  ASSERT_FALSE(ec);
  ec = add(s, "fox");
  ASSERT_FALSE(ec);
  ec = add(s, "jumps");
  ASSERT_FALSE(ec);
  ec = add(s, "over");
  ASSERT_FALSE(ec);
  ec = add(s, "the");
  ASSERT_FALSE(ec);
  ec = add(s, "lazy");
  ASSERT_FALSE(ec);
  ec = add(s, "dog");
  ASSERT_FALSE(ec);
  ASSERT_EQ(s.next_sequence_number(), 10u);

  std::vector<Message> v;
  constexpr auto first = 1;
  constexpr auto last = 9;
  ec = s.get(first, last, v);
  ASSERT_FALSE(ec);
  assert_messages(v);
}

TEST(File_store, open_non_empty_file) {
  File_store s(filename);
  auto ec = s.open();
  ASSERT_FALSE(ec);
  ASSERT_EQ(s.next_sequence_number(), 10u);

  {
    std::vector<Message> v;
    constexpr auto first = 1;
    constexpr auto last = 9;
    ec = s.get(first, last, v);
    ASSERT_FALSE(ec);
    assert_messages(v);
  }

  {
    std::vector<Message> v;
    constexpr auto first = 0; // First out of range.
    constexpr auto last = 9;
    ec = s.get(first, last, v);
    ASSERT_TRUE(ec);
    ASSERT_TRUE(v.empty());
  }
  {
    std::vector<Message> v;
    constexpr auto first = 10; // First out of range.
    constexpr auto last = 9;
    ec = s.get(first, last, v);
    ASSERT_TRUE(ec);
    ASSERT_TRUE(v.empty());
  }
  {
    std::vector<Message> v;
    constexpr auto first = 1;
    constexpr auto last = 0; // Last out of range.
    ec = s.get(first, last, v);
    ASSERT_TRUE(ec);
    ASSERT_TRUE(v.empty());
  }
  {
    std::vector<Message> v;
    constexpr auto first = 1;
    constexpr auto last = 10; // Last out of range.
    ec = s.get(first, last, v);
    ASSERT_TRUE(ec);
    ASSERT_TRUE(v.empty());
  }
  {
    std::vector<Message> v;
    constexpr auto first = 6;
    constexpr auto last = 4; // Last less than first.
    ec = s.get(first, last, v);
    ASSERT_TRUE(ec);
    ASSERT_TRUE(v.empty());
  }

  {
    std::vector<Message> v;
    constexpr auto first = 4;
    constexpr auto last = 6;
    ec = s.get(first, last, v);
    ASSERT_FALSE(ec);
    ASSERT_EQ(v.size(), 3u);
    auto i = v.begin();
    ASSERT_EQ(i->size(), 3u);
    ASSERT_EQ(std::memcmp(i->data(), "fox", i->size()), 0);
    ++i;
    ASSERT_EQ(i->size(), 5u);
    ASSERT_EQ(std::memcmp(i->data(), "jumps", i->size()), 0);
    ++i;
    ASSERT_EQ(i->size(), 4u);
    ASSERT_EQ(std::memcmp(i->data(), "over", i->size()), 0);
    ++i;
    ASSERT_EQ(i, v.end());
  }
}

TEST(File_store, add_to_existing_file) {
  unlink(filename.c_str());
  {
    File_store s;
    s.set_filename(filename);
    auto ec = s.open();
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 1u);

    ec = add(s, "The");
    ASSERT_FALSE(ec);
    ec = add(s, "quick");
    ASSERT_FALSE(ec);
    ec = add(s, "brown");
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 4u);

    std::vector<Message> v;
    constexpr auto first = 1;
    constexpr auto last = 3;
    ec = s.get(first, last, v);
    ASSERT_FALSE(ec);
    ASSERT_EQ(v.size(), 3u);

    ec = s.sync();
    ASSERT_FALSE(ec);
    ec = s.close();
    ASSERT_FALSE(ec);
  }
  {
    File_store s;
    s.set_filename(filename);
    auto ec = s.open();
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 4u);

    ec = add(s, "fox");
    ASSERT_FALSE(ec);
    ec = add(s, "jumps");
    ASSERT_FALSE(ec);
    ec = add(s, "over");
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 7u);

    std::vector<Message> v;
    constexpr auto first = 1;
    constexpr auto last = 6;
    ec = s.get(first, last, v);
    ASSERT_FALSE(ec);
    ASSERT_EQ(v.size(), 6u);

    ec = s.sync();
    ASSERT_FALSE(ec);
    ec = s.close();
    ASSERT_FALSE(ec);
  }
  {
    File_store s;
    s.set_filename(filename);
    auto ec = s.open();
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 7u);

    ec = add(s, "the");
    ASSERT_FALSE(ec);
    ec = add(s, "lazy");
    ASSERT_FALSE(ec);
    ec = add(s, "dog");
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 10u);

    std::vector<Message> v;
    constexpr auto first = 1;
    constexpr auto last = 9;
    ec = s.get(first, last, v);
    ASSERT_FALSE(ec);
    assert_messages(v);

    ec = s.sync();
    ASSERT_FALSE(ec);
    ec = s.close();
    ASSERT_FALSE(ec);
  }
}

TEST(File_store, move_operations) {
  unlink(filename.c_str());
  unlink(filename_2.c_str());

  File_store s1(filename);
  auto ec = s1.open();
  ASSERT_FALSE(ec);
  ASSERT_EQ(s1.next_sequence_number(), 1u);

  ec = add(s1, "The");
  ASSERT_FALSE(ec);
  ec = add(s1, "quick");
  ASSERT_FALSE(ec);
  ec = add(s1, "brown");
  ASSERT_FALSE(ec);
  ASSERT_EQ(s1.next_sequence_number(), 4u);

  File_store s2(std::move(s1));
  ASSERT_EQ(s1.next_sequence_number(), 1u);
  ASSERT_EQ(s2.next_sequence_number(), 4u);

  ec = add(s2, "fox");
  ASSERT_FALSE(ec);
  ec = add(s2, "jumps");
  ASSERT_FALSE(ec);
  ec = add(s2, "over");
  ASSERT_FALSE(ec);
  ASSERT_EQ(s2.next_sequence_number(), 7u);

  File_store s3;
  s3 = std::move(s2);
  ASSERT_EQ(s2.next_sequence_number(), 1u);
  ASSERT_EQ(s3.next_sequence_number(), 7u);

  File_store s4(filename_2);
  ec = s4.open();
  ASSERT_FALSE(ec);
  ASSERT_EQ(s4.next_sequence_number(), 1u);

  ec = add(s4, "The");
  ASSERT_FALSE(ec);
  ec = add(s4, "five");
  ASSERT_FALSE(ec);
  ec = add(s4, "boxing");
  ASSERT_FALSE(ec);
  ec = add(s4, "wizards");
  ASSERT_FALSE(ec);
  ec = add(s4, "jump");
  ASSERT_FALSE(ec);
  ec = add(s4, "quickly");
  ASSERT_FALSE(ec);
  ASSERT_EQ(s4.next_sequence_number(), 7u);

  s4 = std::move(s3);
  ASSERT_EQ(s3.next_sequence_number(), 1u);
  ASSERT_EQ(s4.next_sequence_number(), 7u);

  ec = add(s4, "the");
  ASSERT_FALSE(ec);
  ec = add(s4, "lazy");
  ASSERT_FALSE(ec);
  ec = add(s4, "dog");
  ASSERT_FALSE(ec);
  ASSERT_EQ(s4.next_sequence_number(), 10u);

  std::vector<Message> v;
  constexpr auto first = 1;
  constexpr auto last = 9;
  ec = s4.get(first, last, v);
  ASSERT_FALSE(ec);
  assert_messages(v);

  ec = s1.close();
  ASSERT_FALSE(ec);
  ec = s2.close();
  ASSERT_FALSE(ec);
  ec = s3.close();
  ASSERT_FALSE(ec);
  ec = s4.close();
  ASSERT_FALSE(ec);
}
