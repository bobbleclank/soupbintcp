#include "bc/soup/validate.h"

#include "bc/soup/error.h"

#include <string>

#include <gtest/gtest.h>

using namespace bc::soup;

TEST(validate, validate_username) {
  constexpr int max_input_len = 20;
  constexpr int max_valid_len = 6;

  ASSERT_FALSE(validate_username(""));

  for (int i = 1; i != max_input_len + 1; ++i) {
    auto ec = validate_username(std::string(i, 'a'));
    if (i <= max_valid_len)
      ASSERT_FALSE(ec);
    else
      ASSERT_EQ(ec, Error::username_too_long);
  }

  for (int i = 1; i != max_input_len + 1; ++i) {
    auto ec = validate_username(std::string(i, '1'));
    if (i <= max_valid_len)
      ASSERT_FALSE(ec);
    else
      ASSERT_EQ(ec, Error::username_too_long);
  }

  ASSERT_EQ(validate_username("a!b"), Error::invalid_username);
  ASSERT_EQ(validate_username("a b"), Error::invalid_username);
}

TEST(validate, validate_password) {
  constexpr int max_input_len = 20;
  constexpr int max_valid_len = 10;

  ASSERT_FALSE(validate_password(""));

  for (int i = 1; i != max_input_len + 1; ++i) {
    auto ec = validate_password(std::string(i, 'a'));
    if (i <= max_valid_len)
      ASSERT_FALSE(ec);
    else
      ASSERT_EQ(ec, Error::password_too_long);
  }

  for (int i = 1; i != max_input_len + 1; ++i) {
    auto ec = validate_password(std::string(i, '1'));
    if (i <= max_valid_len)
      ASSERT_FALSE(ec);
    else
      ASSERT_EQ(ec, Error::password_too_long);
  }

  ASSERT_EQ(validate_password("a!b"), Error::invalid_password);
  ASSERT_EQ(validate_password("a b"), Error::invalid_password);
}

TEST(validate, validate_session) {
  constexpr int max_input_len = 20;
  constexpr int max_valid_len = 10;

  ASSERT_FALSE(validate_session(""));

  for (int i = 1; i != max_input_len + 1; ++i) {
    auto ec = validate_session(std::string(i, 'a'));
    if (i <= max_valid_len)
      ASSERT_FALSE(ec);
    else
      ASSERT_EQ(ec, Error::session_too_long);
  }

  for (int i = 1; i != max_input_len + 1; ++i) {
    auto ec = validate_session(std::string(i, '1'));
    if (i <= max_valid_len)
      ASSERT_FALSE(ec);
    else
      ASSERT_EQ(ec, Error::session_too_long);
  }

  ASSERT_EQ(validate_session("a!b"), Error::invalid_session);
  ASSERT_EQ(validate_session("a b"), Error::invalid_session);
}
