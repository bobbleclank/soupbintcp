#include "bc/soup/validate.h"

#include <string>

#include <gtest/gtest.h>

using namespace bc::soup;

TEST(validate, is_valid_username) {
  constexpr int max_input_len = 20;
  constexpr int max_valid_len = 6;

  ASSERT_TRUE(is_valid_username(""));

  for (int i = 1; i != max_input_len + 1; ++i) {
    bool valid = is_valid_username(std::string(i, 'a'));
    if (i <= max_valid_len)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  for (int i = 1; i != max_input_len + 1; ++i) {
    bool valid = is_valid_username(std::string(i, '1'));
    if (i <= max_valid_len)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  ASSERT_FALSE(is_valid_username("a!b"));
  ASSERT_FALSE(is_valid_username("a b"));
}

TEST(validate, is_valid_password) {
  constexpr int max_input_len = 20;
  constexpr int max_valid_len = 10;

  ASSERT_TRUE(is_valid_password(""));

  for (int i = 1; i != max_input_len + 1; ++i) {
    bool valid = is_valid_password(std::string(i, 'a'));
    if (i <= max_valid_len)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  for (int i = 1; i != max_input_len + 1; ++i) {
    bool valid = is_valid_password(std::string(i, '1'));
    if (i <= max_valid_len)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  ASSERT_FALSE(is_valid_password("a!b"));
  ASSERT_FALSE(is_valid_password("a b"));
}

TEST(validate, is_valid_session) {
  constexpr int max_input_len = 20;
  constexpr int max_valid_len = 10;

  ASSERT_TRUE(is_valid_session(""));

  for (int i = 1; i != max_input_len + 1; ++i) {
    bool valid = is_valid_session(std::string(i, 'a'));
    if (i <= max_valid_len)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  for (int i = 1; i != max_input_len + 1; ++i) {
    bool valid = is_valid_session(std::string(i, '1'));
    if (i <= max_valid_len)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  ASSERT_FALSE(is_valid_session("a!b"));
  ASSERT_FALSE(is_valid_session("a b"));
}
