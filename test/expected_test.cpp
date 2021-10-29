#include "exp/expected.h"

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

using namespace exp;

TEST(bad_expected_access, constructor) {
  bad_expected_access<int> e(1);
  ASSERT_STREQ(e.what(), "bad expected access");
  ASSERT_EQ(e.error(), 1);
}

TEST(unexpected, in_place_constructor) {
  unexpected<std::tuple<int, int>> e(std::in_place, 1, 2);
  ASSERT_EQ(e.value(), std::tuple(1, 2));
}

TEST(unexpected, equality_operators) {
  unexpected<int> e_one(std::in_place, 1);
  unexpected<int> e1(std::in_place, 1);
  unexpected<int> e2(std::in_place, 2);

  ASSERT_TRUE(e_one == e1);
  ASSERT_FALSE(e_one == e2);
  ASSERT_FALSE(e_one != e1);
  ASSERT_TRUE(e_one != e2);
}

TEST(expected, default_constructor) {
  expected<std::string, int> e;
  ASSERT_TRUE(static_cast<bool>(e));
  ASSERT_TRUE(e.has_value());
  ASSERT_EQ(e->size(), 0);
  ASSERT_EQ(*e, std::string());
  ASSERT_NO_THROW(e.value());
  ASSERT_EQ(e.value(), std::string());
}

TEST(expected, in_place_constructor) {
  expected<std::string, int> e(std::in_place, "hello world", 5);
  ASSERT_TRUE(static_cast<bool>(e));
  ASSERT_TRUE(e.has_value());
  ASSERT_EQ(e->size(), 5);
  ASSERT_EQ(*e, std::string("hello"));
  ASSERT_NO_THROW(e.value());
  ASSERT_EQ(e.value(), std::string("hello"));
}

TEST(expected, unexpect_constructor) {
  expected<int, std::tuple<int, int>> e(unexpect, 1, 2);
  ASSERT_FALSE(static_cast<bool>(e));
  ASSERT_FALSE(e.has_value());
  ASSERT_EQ(e.error(), std::tuple(1, 2));

  using exception = bad_expected_access<std::tuple<int, int>>;
  ASSERT_THROW(e.value(), exception);
  bool did_throw = false;
  try {
    e.value();
  } catch (const exception& ex) {
    ASSERT_EQ(ex.error(), std::tuple(1, 2));
    did_throw = true;
  }
  ASSERT_TRUE(did_throw);
}

TEST(expected, copy_constructor) {
  {
    expected<std::string, int> other(std::in_place, "hello");
    expected<std::string, int> e(other);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(*e, std::string("hello"));
  }
  {
    expected<std::string, int> other(unexpect, 1);
    expected<std::string, int> e(other);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error(), 1);
  }
}

TEST(expected, move_constructor) {
  {
    expected<std::string, std::vector<int>> other(std::in_place, "hello");
    expected<std::string, std::vector<int>> e(std::move(other));
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(*e, std::string("hello"));
    ASSERT_TRUE(other.has_value());
    ASSERT_EQ(*other, std::string()); // Moved from string.
  }
  {
    expected<std::string, std::vector<int>> other(unexpect, 3, 1);
    expected<std::string, std::vector<int>> e(std::move(other));
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error(), (std::vector{1, 1, 1}));
    ASSERT_FALSE(other.has_value());
    ASSERT_EQ(other.error(), std::vector<int>()); // Moved from vector.
  }
}

TEST(expected, copy_assignment_operator) {
  {
    expected<std::string, int> other(std::in_place, "hello");
    expected<std::string, int> e(std::in_place, "world");
    e = other;
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(*e, std::string("hello"));
  }
  {
    expected<std::string, int> other(std::in_place, "hello");
    expected<std::string, int> e(unexpect, 2);
    e = other;
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(*e, std::string("hello"));
  }
  {
    expected<std::string, int> other(unexpect, 1);
    expected<std::string, int> e(unexpect, 2);
    e = other;
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error(), 1);
  }
  {
    expected<std::string, int> other(unexpect, 1);
    expected<std::string, int> e(std::in_place, "world");
    e = other;
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error(), 1);
  }
}

TEST(expected, move_assignment_operator) {
  {
    expected<std::string, std::vector<int>> other(std::in_place, "hello");
    expected<std::string, std::vector<int>> e(std::in_place, "world");
    e = std::move(other);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(*e, std::string("hello"));
    ASSERT_TRUE(other.has_value());
    ASSERT_EQ(*other, std::string()); // Moved from string.
  }
  {
    expected<std::string, std::vector<int>> other(std::in_place, "hello");
    expected<std::string, std::vector<int>> e(unexpect, 3, 2);
    e = std::move(other);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(*e, std::string("hello"));
    ASSERT_TRUE(other.has_value());
    ASSERT_EQ(*other, std::string()); // Moved from string.
  }
  {
    expected<std::string, std::vector<int>> other(unexpect, 3, 1);
    expected<std::string, std::vector<int>> e(unexpect, 3, 2);
    e = std::move(other);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error(), (std::vector{1, 1, 1}));
    ASSERT_FALSE(other.has_value());
    ASSERT_EQ(other.error(), std::vector<int>()); // Moved from vector.
  }
  {
    expected<std::string, std::vector<int>> other(unexpect, 3, 1);
    expected<std::string, std::vector<int>> e(std::in_place, "world");
    e = std::move(other);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error(), (std::vector{1, 1, 1}));
    ASSERT_FALSE(other.has_value());
    ASSERT_EQ(other.error(), std::vector<int>()); // Moved from vector.
  }
}

TEST(expected, equality_operators) {
  expected<int, int> e_one(std::in_place, 1);
  expected<int, int> u_one(unexpect, 1);
  expected<int, int> e1(std::in_place, 1);
  expected<int, int> e2(std::in_place, 2);
  expected<int, int> u1(unexpect, 1);
  expected<int, int> u2(unexpect, 2);

  ASSERT_TRUE(e_one == e1);
  ASSERT_FALSE(e_one == e2);
  ASSERT_FALSE(e_one != e1);
  ASSERT_TRUE(e_one != e2);

  ASSERT_TRUE(u_one == u1);
  ASSERT_FALSE(u_one == u2);
  ASSERT_FALSE(u_one != u1);
  ASSERT_TRUE(u_one != u2);

  ASSERT_FALSE(e_one == u1);
  ASSERT_FALSE(e_one == u2);
  ASSERT_TRUE(e_one != u1);
  ASSERT_TRUE(e_one != u2);

  ASSERT_FALSE(u_one == e1);
  ASSERT_FALSE(u_one == e2);
  ASSERT_TRUE(u_one != e1);
  ASSERT_TRUE(u_one != e2);
}

TEST(expected, comparison_with_T) {
  expected<int, int> e_one(std::in_place, 1);
  expected<int, int> u_one(unexpect, 1);
  int v1 = 1;
  int v2 = 2;

  ASSERT_TRUE(e_one == v1);
  ASSERT_FALSE(e_one == v2);
  ASSERT_FALSE(e_one != v1);
  ASSERT_TRUE(e_one != v2);

  ASSERT_FALSE(u_one == v1);
  ASSERT_FALSE(u_one == v2);
  ASSERT_TRUE(u_one != v1);
  ASSERT_TRUE(u_one != v2);

  ASSERT_TRUE(v1 == e_one);
  ASSERT_FALSE(v2 == e_one);
  ASSERT_FALSE(v1 != e_one);
  ASSERT_TRUE(v2 != e_one);

  ASSERT_FALSE(v1 == u_one);
  ASSERT_FALSE(v2 == u_one);
  ASSERT_TRUE(v1 != u_one);
  ASSERT_TRUE(v2 != u_one);
}

TEST(expected, comparison_with_unexpected_E) {
  expected<int, int> e_one(std::in_place, 1);
  expected<int, int> u_one(unexpect, 1);
  unexpected<int> v1(std::in_place, 1);
  unexpected<int> v2(std::in_place, 2);

  ASSERT_TRUE(u_one == v1);
  ASSERT_FALSE(u_one == v2);
  ASSERT_FALSE(u_one != v1);
  ASSERT_TRUE(u_one != v2);

  ASSERT_FALSE(e_one == v1);
  ASSERT_FALSE(e_one == v2);
  ASSERT_TRUE(e_one != v1);
  ASSERT_TRUE(e_one != v2);

  ASSERT_TRUE(v1 == u_one);
  ASSERT_FALSE(v2 == u_one);
  ASSERT_FALSE(v1 != u_one);
  ASSERT_TRUE(v2 != u_one);

  ASSERT_FALSE(v1 == e_one);
  ASSERT_FALSE(v2 == e_one);
  ASSERT_TRUE(v1 != e_one);
  ASSERT_TRUE(v2 != e_one);
}
