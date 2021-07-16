#include "option_error.h"

#include <gtest/gtest.h>

TEST(option_error, Illegal_option) {
  Illegal_option e('a');
  ASSERT_STREQ(e.what(), "illegal option \'a\'");
}

TEST(option_error, Missing_argument) {
  Missing_argument e('a');
  ASSERT_STREQ(e.what(), "option \'a\' requires an argument");
}

TEST(option_error, Invalid_argument) {
  {
    Invalid_argument e('a');
    ASSERT_STREQ(e.what(), "option \'a\' has an invalid argument");
  }
  {
    Invalid_argument e('a', "bad value");
    ASSERT_STREQ(e.what(), "option \'a\' has an invalid argument (bad value)");
  }
}
