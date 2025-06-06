#include "bc/soup/error.h"

#include <string>
#include <system_error>

#include <gtest/gtest.h>

using namespace bc::soup;

using namespace std::string_literals;

TEST(error, Error) {
  ASSERT_EQ(static_cast<int>(Error::username_too_long), 1);
  ASSERT_EQ(static_cast<int>(Error::invalid_username), 2);
  ASSERT_EQ(static_cast<int>(Error::password_too_long), 3);
  ASSERT_EQ(static_cast<int>(Error::invalid_password), 4);
  ASSERT_EQ(static_cast<int>(Error::session_too_long), 5);
  ASSERT_EQ(static_cast<int>(Error::invalid_session), 6);
}

TEST(error, souptcp_category) {
  ASSERT_STREQ(souptcp_category().name(), "souptcp");

  ASSERT_EQ(souptcp_category().message(1), "username too long"s);
  ASSERT_EQ(souptcp_category().message(2), "invalid username"s);
  ASSERT_EQ(souptcp_category().message(3), "password too long"s);
  ASSERT_EQ(souptcp_category().message(4), "invalid password"s);
  ASSERT_EQ(souptcp_category().message(5), "session too long"s);
  ASSERT_EQ(souptcp_category().message(6), "invalid session"s);

  ASSERT_EQ(souptcp_category().message(0), "unknown error"s);
  ASSERT_EQ(souptcp_category().message(7), "unknown error"s);

  {
    constexpr int ev = 1;
    std::error_condition ec = souptcp_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 2;
    std::error_condition ec = souptcp_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 3;
    std::error_condition ec = souptcp_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 4;
    std::error_condition ec = souptcp_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 5;
    std::error_condition ec = souptcp_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 6;
    std::error_condition ec = souptcp_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
}

TEST(error, make_error_code) {
  {
    std::error_code ec = make_error_code(Error::username_too_long);
    ASSERT_EQ(ec.value(), 1);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = make_error_code(Error::invalid_username);
    ASSERT_EQ(ec.value(), 2);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = make_error_code(Error::password_too_long);
    ASSERT_EQ(ec.value(), 3);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = make_error_code(Error::invalid_password);
    ASSERT_EQ(ec.value(), 4);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = make_error_code(Error::session_too_long);
    ASSERT_EQ(ec.value(), 5);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = make_error_code(Error::invalid_session);
    ASSERT_EQ(ec.value(), 6);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
}

TEST(error, make_error_condition) {
  {
    std::error_condition ec = make_error_condition(Error::username_too_long);
    ASSERT_EQ(ec.value(), 1);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_condition ec = make_error_condition(Error::invalid_username);
    ASSERT_EQ(ec.value(), 2);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_condition ec = make_error_condition(Error::password_too_long);
    ASSERT_EQ(ec.value(), 3);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_condition ec = make_error_condition(Error::invalid_password);
    ASSERT_EQ(ec.value(), 4);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_condition ec = make_error_condition(Error::session_too_long);
    ASSERT_EQ(ec.value(), 5);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_condition ec = make_error_condition(Error::invalid_session);
    ASSERT_EQ(ec.value(), 6);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
}

TEST(error, is_error_code_enum) {
  ASSERT_TRUE(std::is_error_code_enum<Error>::value);
  {
    std::error_code ec = Error::username_too_long;
    ASSERT_EQ(ec.value(), 1);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = Error::invalid_username;
    ASSERT_EQ(ec.value(), 2);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = Error::password_too_long;
    ASSERT_EQ(ec.value(), 3);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = Error::invalid_password;
    ASSERT_EQ(ec.value(), 4);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = Error::session_too_long;
    ASSERT_EQ(ec.value(), 5);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
  {
    std::error_code ec = Error::invalid_session;
    ASSERT_EQ(ec.value(), 6);
    ASSERT_EQ(ec.category(), souptcp_category());
  }
}

TEST(error, is_error_condition_enum) {
  ASSERT_FALSE(std::is_error_condition_enum<Error>::value);
}

TEST(error, testing_for_an_error) {
  {
    constexpr int ev = 1;
    std::error_code ec(ev, souptcp_category());
    ASSERT_TRUE(ec == Error::username_too_long);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 2;
    std::error_code ec(ev, souptcp_category());
    ASSERT_TRUE(ec == Error::invalid_username);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 3;
    std::error_code ec(ev, souptcp_category());
    ASSERT_TRUE(ec == Error::password_too_long);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 4;
    std::error_code ec(ev, souptcp_category());
    ASSERT_TRUE(ec == Error::invalid_password);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 5;
    std::error_code ec(ev, souptcp_category());
    ASSERT_TRUE(ec == Error::session_too_long);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 6;
    std::error_code ec(ev, souptcp_category());
    ASSERT_TRUE(ec == Error::invalid_session);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
}
