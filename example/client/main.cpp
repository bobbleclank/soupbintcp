#include "bc/soup/client/client.h"
#include "bc_soup_config.h"
#include "io_context_runner.h"
#include "option_convert.h"
#include "option_error.h"

#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <print>
#include <system_error>
#include <thread>

#include <unistd.h>

using namespace bc;
using namespace std::chrono_literals;

class Client {
public:
  explicit Client(asio::io_context& io_context)
      : client_(io_context.get_executor()) {}

  void start() {
    if (const auto ec = client_.start())
      throw std::system_error(ec, "client start");
  }

  void stop() { client_.stop(); }

private:
  soup::client::Client client_;
};

void run(int time) {
  asio::io_context io_context;
  Io_context_runner io_runner(io_context);
  std::atomic<bool> keep_going = true;
  io_runner.set_signal_handler([&keep_going] { keep_going = false; });
  Client client(io_context);

  io_runner.start();
  std::this_thread::sleep_for(1s);
  client.start();

  const auto start = std::chrono::steady_clock::now();
  const auto end = start + std::chrono::seconds(time);
  while (keep_going) {
    const auto now = std::chrono::steady_clock::now();
    if (now >= end)
      break;
    std::this_thread::sleep_for(1s);
  }

  client.stop();
  std::this_thread::sleep_for(1s);
  io_runner.stop();
}

void display_usage() {
  std::print("usage: bc_soup_client [options]\n"
             "options:\n"
             "  -h  help\n"
             "  -t  running time (seconds) [15]\n"
             "  -v  version\n");
}

void display_version() {
  std::println("version {}.{}", bc_soup_VERSION_MAJOR, bc_soup_VERSION_MINOR);
}

int main(int argc, char** argv) {
  constexpr int default_time = 15;
  int time = default_time;

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
    std::println("{}", e.what());
    return EXIT_FAILURE;
  }

  try {
    run(time);
  } catch (const std::system_error& e) {
    std::println("system error: {}:{} {}", e.code().category().name(),
                 e.code().value(), e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
