#include "libsoup_config.h"
#include "option_error.h"
#include "soup/server/server.h"

#include <cstdlib>
#include <iostream>

#include <unistd.h>

void display_usage() {
  std::cout << "usage: soup_server [options]\n"
               "options:\n"
               "  -h  help\n"
               "  -v  version\n";
}

void display_version() {
  std::cout << "version " << libsoup_VERSION_MAJOR << '.'
            << libsoup_VERSION_MINOR << '\n';
}

int main(int argc, char** argv) {
  try {
    int opt = 0;
    while ((opt = getopt(argc, argv, ":hv")) != -1) {
      switch (opt) {
      case 'h':
        display_usage();
        return EXIT_SUCCESS;
      case 'v':
        display_version();
        return EXIT_SUCCESS;
      case ':':
        throw Missing_argument(optopt);
      case '?':
      default:
        throw Illegal_option(optopt);
      }
    }
  } catch (const Option_error& e) {
    std::cout << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
