#include "option_convert.h"

#include "option_error.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>

int to_int(const char* arg, int opt) {
  const long i = to_long(arg, opt);
  if (i < INT_MIN || i > INT_MAX)
    throw Invalid_argument(opt, "out of range");
  return static_cast<int>(i);
}

long to_long(const char* arg, int opt) {
  constexpr auto base = 10;
  errno = 0;
  char* end = nullptr;
  const long i = std::strtol(arg, &end, base);
  if (end == arg)
    throw Invalid_argument(opt, "no conversion");
  if (errno == ERANGE)
    throw Invalid_argument(opt, "out of range");
  if (*end != '\0')
    throw Invalid_argument(opt, "trailing characters");
  return i;
}

Endpoint::Endpoint(int port_) : port(port_) {
}

Endpoint::Endpoint(std::string_view address_, int port_)
    : address(address_), port(port_) {
}

Endpoint to_endpoint(const char* arg, int opt) {
  const auto* ptr = std::strchr(arg, ':');
  if (ptr == nullptr) {
    const auto port = to_int(arg, opt);
    if (port < 0)
      throw Invalid_argument(opt, "negative");
    return Endpoint(port);
  }
  using size_type = std::string_view::size_type;
  const auto len = static_cast<size_type>(ptr - arg);
  const std::string_view address(arg, len);
  const auto port = to_int(ptr + 1, opt);
  if (port < 0)
    throw Invalid_argument(opt, "negative");
  return Endpoint(address, port);
}
