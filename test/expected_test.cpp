#include "bc/soup/expected.h"

#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include <gtest/gtest.h>

using namespace bc::soup;

namespace {

struct Arg {
  explicit Arg(int x_) { x = x_; }

  Arg(const Arg&) = default;

  Arg(Arg&& other) {
    x = other.x;
    other.x = -1;
  }

  Arg& operator=(const Arg&) = delete;
  Arg& operator=(Arg&&) = delete;

  ~Arg() = default;

  int x;
};

struct Obj {
  Obj() { x = 20100; }

  explicit Obj(int x_) { x = x_; }

  explicit Obj(const Arg& arg_) {
    Arg arg = arg_;
    x = arg.x;
  }

  explicit Obj(Arg&& arg_) {
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj(const Obj&) = default;

  Obj(Obj&& other) {
    x = other.x;
    other.x = -1;
  }

  Obj& operator=(const Obj&) = default;

  Obj& operator=(Obj&& other) {
    x = other.x;
    other.x = -2;
    return *this;
  }

  ~Obj() = default;

  int x;
};

struct Obj_implicit {
  explicit Obj_implicit(int x_) { x = x_; }

  Obj_implicit(const Arg& arg_) {
    Arg arg = arg_;
    x = arg.x;
  }

  Obj_implicit(Arg&& arg_) {
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  Obj_implicit(const Obj_implicit&) = default;

  Obj_implicit(Obj_implicit&& other) {
    x = other.x;
    other.x = -1;
  }

  Obj_implicit& operator=(const Obj_implicit&) = delete;
  Obj_implicit& operator=(Obj_implicit&&) = delete;

  ~Obj_implicit() = default;

  int x;
};

struct Obj_explicit {
  explicit Obj_explicit(int x_) { x = x_; }

  explicit Obj_explicit(const Arg& arg_) {
    Arg arg = arg_;
    x = arg.x;
  }

  explicit Obj_explicit(Arg&& arg_) {
    Arg arg = std::move(arg_);
    x = arg.x;
  }

  explicit Obj_explicit(const Obj_explicit&) = default;

  explicit Obj_explicit(Obj_explicit&& other) {
    x = other.x;
    other.x = -1;
  }

  Obj_explicit& operator=(const Obj_explicit&) = delete;
  Obj_explicit& operator=(Obj_explicit&&) = delete;

  ~Obj_explicit() = default;

  int x;
};

} // namespace

TEST(bad_expected_access, constructor) {
  bad_expected_access<int> e(1);
  ASSERT_STREQ(e.what(), "bad expected access");
  ASSERT_EQ(e.error(), 1);
}

TEST(unexpected, value_constructor) {
  // Err = E
  {
    Obj val(3);
    unexpected<Obj> e(val);
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(val.x, 3);
  }
  {
    Obj val(3);
    unexpected<Obj> e(std::move(val));
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(val.x, -1);
  }
  // Err != E
  {
    Arg val(3);
    unexpected<Obj> e(val);
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(val.x, 3);
  }
  {
    Arg val(3);
    unexpected<Obj> e(std::move(val));
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(val.x, -1);
  }
}

TEST(unexpected, in_place_constructor) {
  {
    unexpected<std::tuple<int, int>> e(std::in_place, 1, 2);
    ASSERT_EQ(e.value(), std::tuple(1, 2));
  }
  {
    Arg val(3);
    unexpected<Obj> e(std::in_place, val);
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(val.x, 3);
  }
  {
    Arg val(3);
    unexpected<Obj> e(std::in_place, std::move(val));
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(val.x, -1);
  }
}

TEST(unexpected, deduction_guide) {
  // Via (Err&&) with copy
  {
    Obj val(3);
    unexpected e(val);
    ASSERT_TRUE(
        (std::is_same_v<std::remove_reference_t<decltype(e.value())>, Obj>));
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(val.x, 3);
  }
  // Via (Err&&) with move
  {
    Obj val(3);
    unexpected e(std::move(val));
    ASSERT_TRUE(
        (std::is_same_v<std::remove_reference_t<decltype(e.value())>, Obj>));
    ASSERT_EQ(e.value().x, 3);
    ASSERT_EQ(val.x, -1);
  }
}

TEST(unexpected, equality_operators) {
  unexpected<int> e_one(1);
  unexpected<int> e1(1);
  unexpected<int> e2(2);

  ASSERT_TRUE(e_one == e1);
  ASSERT_FALSE(e_one == e2);
  ASSERT_FALSE(e_one != e1);
  ASSERT_TRUE(e_one != e2);
}

TEST(expected, default_constructor) {
  expected<Obj, int> e;
  ASSERT_TRUE(static_cast<bool>(e));
  ASSERT_TRUE(e.has_value());
  ASSERT_EQ(e->x, 20100);
  ASSERT_EQ((*e).x, 20100);
  ASSERT_NO_THROW(e.value());
  ASSERT_EQ(e.value().x, 20100);
}

TEST(expected, value_constructor) {
  // explicit with U = T
  {
    Obj_explicit val(3);
    expected<Obj_explicit, int> e(val);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, 3);
  }
  {
    Obj_explicit val(3);
    expected<Obj_explicit, int> e(std::move(val));
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, -1);
  }
  // explicit with U != T
  {
    Arg val(3);
    expected<Obj_explicit, int> e(val);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, 3);
  }
  {
    Arg val(3);
    expected<Obj_explicit, int> e(std::move(val));
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, -1);
  }
  // implicit with U = T
  {
    Obj_implicit val(3);
    expected<Obj_implicit, int> e = val;
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, 3);
  }
  {
    Obj_implicit val(3);
    expected<Obj_implicit, int> e = std::move(val);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, -1);
  }
  // implicit with U != T
  {
    Arg val(3);
    expected<Obj_implicit, int> e = val;
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, 3);
  }
  {
    Arg val(3);
    expected<Obj_implicit, int> e = std::move(val);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, -1);
  }
}

TEST(expected, copy_unexpected_constructor) {
  // explicit with G = E
  {
    unexpected<Obj_explicit> val(3);
    expected<int, Obj_explicit> e(val);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.value().x, 3);
  }
  // explicit with G != E
  {
    unexpected<Arg> val(3);
    expected<int, Obj_explicit> e(val);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.value().x, 3);
  }
  // implicit with G = E
  {
    unexpected<Obj_implicit> val(3);
    expected<int, Obj_implicit> e = val;
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.value().x, 3);
  }
  // implicit with G != E
  {
    unexpected<Arg> val(3);
    expected<int, Obj_implicit> e = val;
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.value().x, 3);
  }
}

TEST(expected, move_unexpected_constructor) {
  // explicit with G = E
  {
    unexpected<Obj_explicit> val(3);
    expected<int, Obj_explicit> e(std::move(val));
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.value().x, -1);
  }
  // explicit with G != E
  {
    unexpected<Arg> val(3);
    expected<int, Obj_explicit> e(std::move(val));
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.value().x, -1);
  }
  // implicit with G = E
  {
    unexpected<Obj_implicit> val(3);
    expected<int, Obj_implicit> e = std::move(val);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.value().x, -1);
  }
  // implicit with G != E
  {
    unexpected<Arg> val(3);
    expected<int, Obj_implicit> e = std::move(val);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.value().x, -1);
  }
}

TEST(expected, in_place_constructor) {
  {
    expected<std::string, int> e(std::in_place, "hello world", 5);
    ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(std::string_view(e->data(), e->size()), "hello");
    ASSERT_EQ(*e, "hello");
    ASSERT_NO_THROW(e.value());
    ASSERT_EQ(e.value(), "hello");
  }
  {
    Arg val(3);
    expected<Obj, int> e(std::in_place, val);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, 3);
  }
  {
    Arg val(3);
    expected<Obj, int> e(std::in_place, std::move(val));
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(val.x, -1);
  }
}

TEST(expected, unexpect_constructor) {
  {
    expected<int, std::tuple<int, int>> e(unexpect, 1, 2);
    ASSERT_FALSE(static_cast<bool>(e));
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error(), std::tuple(1, 2));
  }
  {
    Arg val(3);
    expected<int, Obj> e(unexpect, val);
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.x, 3);
  }
  {
    Arg val(3);
    expected<int, Obj> e(unexpect, std::move(val));
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(val.x, -1);
  }
}

TEST(expected, value_bad_access) {
  expected<int, std::tuple<int, int>> e(unexpect, 1, 2);
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
    expected<Obj, int> other(std::in_place, 3);
    expected<Obj, int> e(other);
    ASSERT_TRUE(e.has_value());
    ASSERT_TRUE(other.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(other->x, 3);
  }
  {
    expected<int, Obj> other(unexpect, 3);
    expected<int, Obj> e(other);
    ASSERT_FALSE(e.has_value());
    ASSERT_FALSE(other.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(other.error().x, 3);
  }
}

TEST(expected, move_constructor) {
  {
    expected<Obj, int> other(std::in_place, 3);
    expected<Obj, int> e(std::move(other));
    ASSERT_TRUE(e.has_value());
    ASSERT_TRUE(other.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(other->x, -1);
  }
  {
    expected<int, Obj> other(unexpect, 3);
    expected<int, Obj> e(std::move(other));
    ASSERT_FALSE(e.has_value());
    ASSERT_FALSE(other.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(other.error().x, -1);
  }
}

TEST(expected, copy_assignment_operator) {
  {
    expected<Obj, int> other(std::in_place, 3);
    expected<Obj, int> e(std::in_place, 30);
    e = other;
    ASSERT_TRUE(e.has_value());
    ASSERT_TRUE(other.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(other->x, 3);
  }
  {
    expected<Obj, int> other(std::in_place, 3);
    expected<Obj, int> e(unexpect, 30);
    e = other;
    ASSERT_TRUE(e.has_value());
    ASSERT_TRUE(other.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(other->x, 3);
  }
  {
    expected<int, Obj> other(unexpect, 3);
    expected<int, Obj> e(unexpect, 30);
    e = other;
    ASSERT_FALSE(e.has_value());
    ASSERT_FALSE(other.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(other.error().x, 3);
  }
  {
    expected<int, Obj> other(unexpect, 3);
    expected<int, Obj> e(std::in_place, 30);
    e = other;
    ASSERT_FALSE(e.has_value());
    ASSERT_FALSE(other.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(other.error().x, 3);
  }
}

TEST(expected, move_assignment_operator) {
  {
    expected<Obj, int> other(std::in_place, 3);
    expected<Obj, int> e(std::in_place, 30);
    e = std::move(other);
    ASSERT_TRUE(e.has_value());
    ASSERT_TRUE(other.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(other->x, -2);
  }
  {
    expected<Obj, int> other(std::in_place, 3);
    expected<Obj, int> e(unexpect, 30);
    e = std::move(other);
    ASSERT_TRUE(e.has_value());
    ASSERT_TRUE(other.has_value());
    ASSERT_EQ(e->x, 3);
    ASSERT_EQ(other->x, -1);
  }
  {
    expected<int, Obj> other(unexpect, 3);
    expected<int, Obj> e(unexpect, 30);
    e = std::move(other);
    ASSERT_FALSE(e.has_value());
    ASSERT_FALSE(other.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(other.error().x, -2);
  }
  {
    expected<int, Obj> other(unexpect, 3);
    expected<int, Obj> e(std::in_place, 30);
    e = std::move(other);
    ASSERT_FALSE(e.has_value());
    ASSERT_FALSE(other.has_value());
    ASSERT_EQ(e.error().x, 3);
    ASSERT_EQ(other.error().x, -1);
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
  unexpected<int> v1(1);
  unexpected<int> v2(2);

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
