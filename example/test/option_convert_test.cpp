#include "option_convert.h"

#include "option_error.h"

#include <gtest/gtest.h>

TEST(option_convert, to_int) {
  ASSERT_EQ(to_int("0", 'a'), 0);
  ASSERT_EQ(to_int("1", 'a'), 1);
  ASSERT_EQ(to_int("2", 'a'), 2);
  ASSERT_EQ(to_int("2147483646", 'a'), 2147483646);
  ASSERT_EQ(to_int("2147483647", 'a'), 2147483647);

  ASSERT_EQ(to_int("-1", 'a'), -1);
  ASSERT_EQ(to_int("-2", 'a'), -2);
  ASSERT_EQ(to_int("-2147483647", 'a'), -2147483647);
  ASSERT_EQ(to_int("-2147483648", 'a'), -2147483647 - 1);

  ASSERT_THROW(to_int("2147483648", 'a'), Invalid_argument);
  try {
    to_int("2147483648", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (out of range)");
  }

  ASSERT_THROW(to_int("-2147483649", 'a'), Invalid_argument);
  try {
    to_int("-2147483649", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (out of range)");
  }

  ASSERT_THROW(to_int("abc123", 'a'), Invalid_argument);
  try {
    to_int("abc123", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (no conversion)");
  }

  ASSERT_THROW(to_int("", 'a'), Invalid_argument);
  try {
    to_int("", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (no conversion)");
  }

  ASSERT_THROW(to_int("123abc", 'a'), Invalid_argument);
  try {
    to_int("123abc", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (trailing characters)");
  }

  ASSERT_THROW(to_int("123   ", 'a'), Invalid_argument);
  try {
    to_int("123   ", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (trailing characters)");
  }
}

TEST(option_convert, to_long) {
  ASSERT_EQ(to_long("0", 'a'), 0L);
  ASSERT_EQ(to_long("1", 'a'), 1L);
  ASSERT_EQ(to_long("2", 'a'), 2L);
  ASSERT_EQ(to_long("2147483646", 'a'), 2147483646L);
  ASSERT_EQ(to_long("2147483647", 'a'), 2147483647L);
  ASSERT_EQ(to_long("2147483648", 'a'), 2147483648L);
  ASSERT_EQ(to_long("2147483649", 'a'), 2147483649L);
  ASSERT_EQ(to_long("9223372036854775806", 'a'), 9223372036854775806L);
  ASSERT_EQ(to_long("9223372036854775807", 'a'), 9223372036854775807L);

  ASSERT_EQ(to_long("-1", 'a'), -1L);
  ASSERT_EQ(to_long("-2", 'a'), -2L);
  ASSERT_EQ(to_long("-2147483647", 'a'), -2147483647L);
  ASSERT_EQ(to_long("-2147483648", 'a'), -2147483648L);
  ASSERT_EQ(to_long("-2147483649", 'a'), -2147483649L);
  ASSERT_EQ(to_long("-2147483650", 'a'), -2147483650L);
  ASSERT_EQ(to_long("-9223372036854775807", 'a'), -9223372036854775807L);
  ASSERT_EQ(to_long("-9223372036854775808", 'a'), -9223372036854775807L - 1L);

  ASSERT_THROW(to_long("9223372036854775808", 'a'), Invalid_argument);
  try {
    to_long("9223372036854775808", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (out of range)");
  }

  ASSERT_THROW(to_long("-9223372036854775809", 'a'), Invalid_argument);
  try {
    to_long("-9223372036854775809", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (out of range)");
  }

  ASSERT_THROW(to_long("abc123", 'a'), Invalid_argument);
  try {
    to_long("abc123", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (no conversion)");
  }

  ASSERT_THROW(to_long("", 'a'), Invalid_argument);
  try {
    to_long("", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (no conversion)");
  }

  ASSERT_THROW(to_long("123abc", 'a'), Invalid_argument);
  try {
    to_long("123abc", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (trailing characters)");
  }

  ASSERT_THROW(to_long("123   ", 'a'), Invalid_argument);
  try {
    to_long("123   ", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (trailing characters)");
  }
}

TEST(option_convert, to_endpoint) {
  {
    auto ep = to_endpoint("5555", 'a');
    ASSERT_EQ(ep.address.data(), nullptr);
    ASSERT_TRUE(ep.address.empty());
    ASSERT_EQ(ep.port, 5555);
  }

  ASSERT_THROW(to_endpoint("-5555", 'a'), Invalid_argument);
  try {
    to_endpoint("-5555", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(), "option \'a\' has an invalid argument (negative)");
  }

  ASSERT_THROW(to_endpoint("abc", 'a'), Invalid_argument);
  try {
    to_endpoint("abc", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (no conversion)");
  }

  ASSERT_THROW(to_endpoint("", 'a'), Invalid_argument);
  try {
    to_endpoint("", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (no conversion)");
  }

  ASSERT_THROW(to_endpoint("555five", 'a'), Invalid_argument);
  try {
    to_endpoint("555five", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (trailing characters)");
  }

  {
    auto ep = to_endpoint(":5555", 'a');
    ASSERT_NE(ep.address.data(), nullptr);
    ASSERT_TRUE(ep.address.empty());
    ASSERT_EQ(ep.port, 5555);
  }

  ASSERT_THROW(to_endpoint(":-5555", 'a'), Invalid_argument);
  try {
    to_endpoint(":-5555", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(), "option \'a\' has an invalid argument (negative)");
  }

  ASSERT_THROW(to_endpoint(":abc", 'a'), Invalid_argument);
  try {
    to_endpoint(":abc", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (no conversion)");
  }

  ASSERT_THROW(to_endpoint(":", 'a'), Invalid_argument);
  try {
    to_endpoint(":", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (no conversion)");
  }

  ASSERT_THROW(to_endpoint(":555five", 'a'), Invalid_argument);
  try {
    to_endpoint(":555five", 'a');
  } catch (const Invalid_argument& e) {
    ASSERT_STREQ(e.what(),
                 "option \'a\' has an invalid argument (trailing characters)");
  }

  {
    auto ep = to_endpoint("address:5555", 'a');
    ASSERT_NE(ep.address.data(), nullptr);
    ASSERT_FALSE(ep.address.empty());
    ASSERT_EQ(ep.address, "address");
    ASSERT_EQ(ep.port, 5555);
  }
}
