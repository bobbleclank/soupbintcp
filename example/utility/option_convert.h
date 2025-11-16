#ifndef EXAMPLE_UTILITY_OPTION_CONVERT_H
#define EXAMPLE_UTILITY_OPTION_CONVERT_H

#include <string_view>

int to_int(std::string_view, int);
long to_long(std::string_view, int);

struct Endpoint {
  Endpoint() = default;
  explicit Endpoint(int);
  Endpoint(std::string_view, int);

  std::string_view address;
  int port = 0;
};

Endpoint to_endpoint(std::string_view, int);

#endif
