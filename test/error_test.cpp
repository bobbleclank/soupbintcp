#include "bc/soup/error.h"

#include <string>
#include <system_error>

#include <gtest/gtest.h>

using namespace bc::soup;

using namespace std::string_literals;

TEST(error, error_value) {
  ASSERT_EQ(static_cast<int>(Error::username_too_long), 1);
  ASSERT_EQ(static_cast<int>(Error::invalid_username), 2);
  ASSERT_EQ(static_cast<int>(Error::password_too_long), 3);
  ASSERT_EQ(static_cast<int>(Error::invalid_password), 4);
  ASSERT_EQ(static_cast<int>(Error::session_too_long), 5);
  ASSERT_EQ(static_cast<int>(Error::invalid_session), 6);
  ASSERT_EQ(static_cast<int>(Error::endpoint_in_use), 7);
  ASSERT_EQ(static_cast<int>(Error::username_in_use), 8);
  ASSERT_EQ(static_cast<int>(Error::handler_not_set), 9);
}

TEST(error, soup_category) {
  ASSERT_STREQ(soup_category().name(), "soup");

  ASSERT_EQ(soup_category().message(1), "username too long"s);
  ASSERT_EQ(soup_category().message(2), "invalid username"s);
  ASSERT_EQ(soup_category().message(3), "password too long"s);
  ASSERT_EQ(soup_category().message(4), "invalid password"s);
  ASSERT_EQ(soup_category().message(5), "session too long"s);
  ASSERT_EQ(soup_category().message(6), "invalid session"s);
  ASSERT_EQ(soup_category().message(7), "endpoint in use"s);
  ASSERT_EQ(soup_category().message(8), "username in use"s);
  ASSERT_EQ(soup_category().message(9), "handler not set"s);

  ASSERT_EQ(soup_category().message(0), "unknown error"s);
  ASSERT_EQ(soup_category().message(10), "unknown error"s);

  {
    constexpr int ev = 1;
    std::error_condition ec = soup_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 2;
    std::error_condition ec = soup_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 3;
    std::error_condition ec = soup_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 4;
    std::error_condition ec = soup_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 5;
    std::error_condition ec = soup_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 6;
    std::error_condition ec = soup_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::invalid_argument));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 7;
    std::error_condition ec = soup_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), static_cast<int>(std::errc::address_in_use));
    ASSERT_EQ(ec.category(), std::generic_category());
  }
  {
    constexpr int ev = 8;
    std::error_condition ec = soup_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), ev);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    constexpr int ev = 9;
    std::error_condition ec = soup_category().default_error_condition(ev);
    ASSERT_EQ(ec.value(), ev);
    ASSERT_EQ(ec.category(), soup_category());
  }
}

TEST(error, make_error_code) {
  {
    std::error_code ec = make_error_code(Error::username_too_long);
    ASSERT_EQ(ec.value(), 1);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = make_error_code(Error::invalid_username);
    ASSERT_EQ(ec.value(), 2);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = make_error_code(Error::password_too_long);
    ASSERT_EQ(ec.value(), 3);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = make_error_code(Error::invalid_password);
    ASSERT_EQ(ec.value(), 4);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = make_error_code(Error::session_too_long);
    ASSERT_EQ(ec.value(), 5);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = make_error_code(Error::invalid_session);
    ASSERT_EQ(ec.value(), 6);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = make_error_code(Error::endpoint_in_use);
    ASSERT_EQ(ec.value(), 7);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = make_error_code(Error::username_in_use);
    ASSERT_EQ(ec.value(), 8);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = make_error_code(Error::handler_not_set);
    ASSERT_EQ(ec.value(), 9);
    ASSERT_EQ(ec.category(), soup_category());
  }
}

TEST(error, is_error_code_enum) {
  ASSERT_TRUE(std::is_error_code_enum<Error>::value);
  {
    std::error_code ec = Error::username_too_long;
    ASSERT_EQ(ec.value(), 1);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = Error::invalid_username;
    ASSERT_EQ(ec.value(), 2);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = Error::password_too_long;
    ASSERT_EQ(ec.value(), 3);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = Error::invalid_password;
    ASSERT_EQ(ec.value(), 4);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = Error::session_too_long;
    ASSERT_EQ(ec.value(), 5);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = Error::invalid_session;
    ASSERT_EQ(ec.value(), 6);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = Error::endpoint_in_use;
    ASSERT_EQ(ec.value(), 7);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = Error::username_in_use;
    ASSERT_EQ(ec.value(), 8);
    ASSERT_EQ(ec.category(), soup_category());
  }
  {
    std::error_code ec = Error::handler_not_set;
    ASSERT_EQ(ec.value(), 9);
    ASSERT_EQ(ec.category(), soup_category());
  }
}

TEST(error, is_error_condition_enum) {
  ASSERT_FALSE(std::is_error_condition_enum<Error>::value);
}

TEST(error, comparison) {
  {
    constexpr int ev = 1;
    std::error_code ec(ev, soup_category());
    ASSERT_TRUE(ec == Error::username_too_long);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 2;
    std::error_code ec(ev, soup_category());
    ASSERT_TRUE(ec == Error::invalid_username);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 3;
    std::error_code ec(ev, soup_category());
    ASSERT_TRUE(ec == Error::password_too_long);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 4;
    std::error_code ec(ev, soup_category());
    ASSERT_TRUE(ec == Error::invalid_password);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 5;
    std::error_code ec(ev, soup_category());
    ASSERT_TRUE(ec == Error::session_too_long);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 6;
    std::error_code ec(ev, soup_category());
    ASSERT_TRUE(ec == Error::invalid_session);
    ASSERT_TRUE(ec == std::errc::invalid_argument);
  }
  {
    constexpr int ev = 7;
    std::error_code ec(ev, soup_category());
    ASSERT_TRUE(ec == Error::endpoint_in_use);
    ASSERT_TRUE(ec == std::errc::address_in_use);
  }
  {
    constexpr int ev = 8;
    std::error_code ec(ev, soup_category());
    ASSERT_TRUE(ec == Error::username_in_use);
  }
  {
    constexpr int ev = 9;
    std::error_code ec(ev, soup_category());
    ASSERT_TRUE(ec == Error::handler_not_set);
  }
}
