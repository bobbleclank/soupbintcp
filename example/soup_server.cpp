#include "bc/soup/server/server.h"
#include "bc_soupbintcp_config.h"
#include "io_context_runner.h"
#include "option_convert.h"
#include "option_error.h"

#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <unistd.h>

using namespace bc;
using namespace std::chrono_literals;

class Server {
public:
  explicit Server(asio::io_context& io_context)
      : server_(io_context.get_executor()) {}

  void start() { server_.start(); }
  void stop() { server_.stop(); }

private:
  soup::server::Server server_;
};

void run(int time) {
  asio::io_context io_context;
  Io_context_runner io_runner(io_context);
  std::atomic<bool> keep_going = true;
  io_runner.set_signal_handler([&keep_going] { keep_going = false; });
  Server server(io_context);

  io_runner.start();
  std::this_thread::sleep_for(1s);
  server.start();

  const auto start = std::chrono::steady_clock::now();
  const auto end = start + std::chrono::seconds(time);
  while (keep_going) {
    const auto now = std::chrono::steady_clock::now();
    if (now >= end)
      break;
    std::this_thread::sleep_for(1s);
  }

  server.stop();
  std::this_thread::sleep_for(1s);
  io_runner.stop();
}

void display_usage() {
  std::cout << "usage: bc_souptcp_server [options]\n"
               "options:\n"
               "  -h  help\n"
               "  -t  running time (seconds) [30]\n"
               "  -v  version\n";
}

void display_version() {
  std::cout << "version " << bc_soupbintcp_VERSION_MAJOR << '.'
            << bc_soupbintcp_VERSION_MINOR << '\n';
}

int main(int argc, char** argv) {
  int time = 30;

  try {
    int opt = 0;
    while ((opt = getopt(argc, argv, ":ht:v")) != -1) {
      switch (opt) {
      case 'h':
        display_usage();
        return EXIT_SUCCESS;
      case 't':
        time = to_int(optarg, optopt);
        break;
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

  run(time);

  return EXIT_SUCCESS;
}
