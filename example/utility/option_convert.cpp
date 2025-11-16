#include "option_convert.h"

#include "option_error.h"

#include <cassert>
#include <charconv>
#include <climits>
#include <system_error>

int to_int(std::string_view arg, int opt) {
  const long i = to_long(arg, opt);
  if (i < INT_MIN || i > INT_MAX)
    throw Invalid_argument(opt, "out of range");
  return static_cast<int>(i);
}

long to_long(std::string_view arg, int opt) {
  long i = 0;
  const auto [ptr, ec] = std::from_chars(arg.begin(), arg.end(), i);
  if (ec != std::errc() || ptr != arg.end()) {
    if (ec == std::errc::invalid_argument)
      throw Invalid_argument(opt, "no conversion");
    if (ec == std::errc::result_out_of_range)
      throw Invalid_argument(opt, "out of range");
    assert(ec == std::errc() && ptr != arg.end());
    throw Invalid_argument(opt, "trailing characters");
  }
  return i;
}

Endpoint::Endpoint(int port_) : port(port_) {
}

Endpoint::Endpoint(std::string_view address_, int port_)
    : address(address_), port(port_) {
}

Endpoint to_endpoint(std::string_view arg, int opt) {
  const auto pos = arg.find(':');
  if (pos == std::string_view::npos) {
    const auto port = to_int(arg, opt);
    if (port < 0)
      throw Invalid_argument(opt, "negative");
    return Endpoint(port);
  }
  const std::string_view address = arg.substr(0, pos);
  arg.remove_prefix(pos + 1);
  const auto port = to_int(arg, opt);
  if (port < 0)
    throw Invalid_argument(opt, "negative");
  return Endpoint(address, port);
}
