#ifndef EXAMPLE_UTILITY_OPTION_ERROR_H
#define EXAMPLE_UTILITY_OPTION_ERROR_H

#include <exception>
#include <string>

class Option_error : public std::exception {
public:
  const char* what() const noexcept override { return what_.c_str(); }

protected:
  explicit Option_error(std::string&&);

private:
  std::string what_;
};

class Illegal_option : public Option_error {
public:
  explicit Illegal_option(int);
};

class Missing_argument : public Option_error {
public:
  explicit Missing_argument(int);
};

class Invalid_argument : public Option_error {
public:
  explicit Invalid_argument(int);
  Invalid_argument(int, const char*);
};

#endif
