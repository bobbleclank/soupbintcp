#include "bc/soup/file_store.h"

#include <cstring>
#include <utility>
#include <vector>

#include <unistd.h>

#include <gtest/gtest.h>

using namespace bc::soup;

namespace {

constexpr char filename[] = "test_store";
constexpr char filename_2[] = "test_store_2";

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
  unlink(filename);

  File_store s(filename);
  auto ec = s.open();
  ASSERT_FALSE(ec);
  ASSERT_EQ(s.next_sequence_number(), 1u);

  ec = s.add("The", 3);
  ASSERT_FALSE(ec);
  ec = s.add("quick", 5);
  ASSERT_FALSE(ec);
  ec = s.add("brown", 5);
  ASSERT_FALSE(ec);
  ec = s.add("fox", 3);
  ASSERT_FALSE(ec);
  ec = s.add("jumps", 5);
  ASSERT_FALSE(ec);
  ec = s.add("over", 4);
  ASSERT_FALSE(ec);
  ec = s.add("the", 3);
  ASSERT_FALSE(ec);
  ec = s.add("lazy", 4);
  ASSERT_FALSE(ec);
  ec = s.add("dog", 3);
  ASSERT_FALSE(ec);
  ASSERT_EQ(s.next_sequence_number(), 10u);

  std::vector<Message> v;
  ec = s.get(1, 9, v);
  ASSERT_FALSE(ec);
  assert_messages(v);
}

TEST(File_store, open_non_empty_file) {
  File_store s(filename);
  auto ec = s.open();
  ASSERT_FALSE(ec);
  ASSERT_EQ(s.next_sequence_number(), 10u);

  std::vector<Message> v;
  ec = s.get(1, 9, v);
  ASSERT_FALSE(ec);
  assert_messages(v);

  v.clear();
  ec = s.get(0, 9, v); // First out of range.
  ASSERT_TRUE(ec);
  ASSERT_TRUE(v.empty());
  ec = s.get(10, 9, v); // First out of range.
  ASSERT_TRUE(ec);
  ASSERT_TRUE(v.empty());
  ec = s.get(1, 0, v); // Last out of range.
  ASSERT_TRUE(ec);
  ASSERT_TRUE(v.empty());
  ec = s.get(1, 10, v); // Last out of range.
  ASSERT_TRUE(ec);
  ASSERT_TRUE(v.empty());
  ec = s.get(6, 4, v); // Invalid range.
  ASSERT_TRUE(ec);
  ASSERT_TRUE(v.empty());

  ec = s.get(4, 6, v);
  ASSERT_FALSE(ec);
  ASSERT_EQ(v.size(), 3u);
  ASSERT_EQ(v[0].size(), 3u);
  ASSERT_EQ(std::memcmp(v[0].data(), "fox", v[0].size()), 0);
  ASSERT_EQ(v[1].size(), 5u);
  ASSERT_EQ(std::memcmp(v[1].data(), "jumps", v[1].size()), 0);
  ASSERT_EQ(v[2].size(), 4u);
  ASSERT_EQ(std::memcmp(v[2].data(), "over", v[2].size()), 0);
}

TEST(File_store, add_to_existing_file) {
  unlink(filename);
  {
    File_store s;
    s.set_filename(filename);
    auto ec = s.open();
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 1u);

    ec = s.add("The", 3);
    ASSERT_FALSE(ec);
    ec = s.add("quick", 5);
    ASSERT_FALSE(ec);
    ec = s.add("brown", 5);
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 4u);

    std::vector<Message> v;
    ec = s.get(1, 3, v);
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

    ec = s.add("fox", 3);
    ASSERT_FALSE(ec);
    ec = s.add("jumps", 5);
    ASSERT_FALSE(ec);
    ec = s.add("over", 4);
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 7u);

    std::vector<Message> v;
    ec = s.get(1, 6, v);
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

    ec = s.add("the", 3);
    ASSERT_FALSE(ec);
    ec = s.add("lazy", 4);
    ASSERT_FALSE(ec);
    ec = s.add("dog", 3);
    ASSERT_FALSE(ec);
    ASSERT_EQ(s.next_sequence_number(), 10u);

    std::vector<Message> v;
    ec = s.get(1, 9, v);
    ASSERT_FALSE(ec);
    assert_messages(v);

    ec = s.sync();
    ASSERT_FALSE(ec);
    ec = s.close();
    ASSERT_FALSE(ec);
  }
}

TEST(File_store, move_operations) {
  unlink(filename);
  unlink(filename_2);

  File_store s1(filename);
  auto ec = s1.open();
  ASSERT_FALSE(ec);
  ASSERT_EQ(s1.next_sequence_number(), 1u);

  ec = s1.add("The", 3);
  ASSERT_FALSE(ec);
  ec = s1.add("quick", 5);
  ASSERT_FALSE(ec);
  ec = s1.add("brown", 5);
  ASSERT_FALSE(ec);
  ASSERT_EQ(s1.next_sequence_number(), 4u);

  File_store s2(std::move(s1));
  ASSERT_EQ(s1.next_sequence_number(), 1u);
  ASSERT_EQ(s2.next_sequence_number(), 4u);

  ec = s2.add("fox", 3);
  ASSERT_FALSE(ec);
  ec = s2.add("jumps", 5);
  ASSERT_FALSE(ec);
  ec = s2.add("over", 4);
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

  ec = s4.add("The", 3);
  ASSERT_FALSE(ec);
  ec = s4.add("five", 4);
  ASSERT_FALSE(ec);
  ec = s4.add("boxing", 6);
  ASSERT_FALSE(ec);
  ec = s4.add("wizards", 7);
  ASSERT_FALSE(ec);
  ec = s4.add("jump", 4);
  ASSERT_FALSE(ec);
  ec = s4.add("quickly", 7);
  ASSERT_FALSE(ec);
  ASSERT_EQ(s4.next_sequence_number(), 7u);

  s4 = std::move(s3);
  ASSERT_EQ(s3.next_sequence_number(), 1u);
  ASSERT_EQ(s4.next_sequence_number(), 7u);

  ec = s4.add("the", 3);
  ASSERT_FALSE(ec);
  ec = s4.add("lazy", 4);
  ASSERT_FALSE(ec);
  ec = s4.add("dog", 3);
  ASSERT_FALSE(ec);
  ASSERT_EQ(s4.next_sequence_number(), 10u);

  std::vector<Message> v;
  ec = s4.get(1, 9, v);
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
