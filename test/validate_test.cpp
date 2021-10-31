#include "bc/soup/validate.h"

#include <string>

#include <gtest/gtest.h>

using namespace bc::soup;

TEST(validate, is_valid_username) {
  ASSERT_TRUE(is_valid_username(""));

  for (int i = 1; i != 21; ++i) {
    bool valid = is_valid_username(std::string(i, 'a'));
    if (i <= 6)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  for (int i = 1; i != 21; ++i) {
    bool valid = is_valid_username(std::string(i, '1'));
    if (i <= 6)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  ASSERT_FALSE(is_valid_username("a!b"));
  ASSERT_FALSE(is_valid_username("a b"));
}

TEST(validate, is_valid_password) {
  ASSERT_TRUE(is_valid_password(""));

  for (int i = 1; i != 21; ++i) {
    bool valid = is_valid_password(std::string(i, 'a'));
    if (i <= 10)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  for (int i = 1; i != 21; ++i) {
    bool valid = is_valid_password(std::string(i, '1'));
    if (i <= 10)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  ASSERT_FALSE(is_valid_password("a!b"));
  ASSERT_FALSE(is_valid_password("a b"));
}

TEST(validate, is_valid_session) {
  ASSERT_TRUE(is_valid_session(""));

  for (int i = 1; i != 21; ++i) {
    bool valid = is_valid_session(std::string(i, 'a'));
    if (i <= 10)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  for (int i = 1; i != 21; ++i) {
    bool valid = is_valid_session(std::string(i, '1'));
    if (i <= 10)
      ASSERT_TRUE(valid);
    else
      ASSERT_FALSE(valid);
  }

  ASSERT_FALSE(is_valid_session("a!b"));
  ASSERT_FALSE(is_valid_session("a b"));
}
