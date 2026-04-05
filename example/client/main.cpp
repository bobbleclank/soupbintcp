#include "bc/soup/client/client.h"
#include "bc/soup/client/connection.h"
#include "bc/soup/client/handler.h"
#include "bc/soup/expected.h"
#include "bc_soup_config.h"
#include "io_context_runner.h"
#include "option_convert.h"
#include "option_error.h"

#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <optional>
#include <print>
#include <system_error>
#include <thread>

#include <unistd.h>

using namespace bc;
using namespace std::chrono_literals;

class Connection final : public soup::client::Connection_handler {
public:
  explicit Connection(soup::client::Connection* connection)
      : connection_(connection) {
    connection_->set_handler(*this);
  }

  void connecting(const asio::ip::tcp::endpoint& ep) override {
    std::println("connecting: endpoint = {}:{}", ep.address().to_string(),
                 ep.port());
  }

  void connection_failure(asio::error_code ec, const char* phase) override {
    std::println("connection failure: error = {}:{} {}, phase = {}",
                 ec.category().name(), ec.value(), ec.message(), phase);
  }

  void connection_success(const asio::ip::tcp::endpoint& local_ep,
                          const asio::ip::tcp::endpoint& remote_ep) override {
    std::println(
        "connection success: local endpoint = {}:{}, remote endpoint = {}:{}",
        local_ep.address().to_string(), local_ep.port(),
        remote_ep.address().to_string(), remote_ep.port());
  }

private:
  soup::client::Connection* connection_ = nullptr;
};

class Client final : public soup::client::Client_handler {
public:
  explicit Client(asio::io_context& io_context)
      : client_(io_context.get_executor()) {
    client_.set_handler(*this);
  }

  void initialize() {
    const auto address = asio::ip::make_address("127.0.0.1");
    const unsigned short port = 5050;
    const asio::ip::tcp::endpoint ep(address, port);
    const auto result = client_.add_connection(ep);
    if (!result)
      throw std::system_error(result.error(), "add connection");
    connection_.emplace(*result);
  }

  void start() {
    if (const auto ec = client_.start())
      throw std::system_error(ec, "client start");
  }

  void stop() { client_.stop(); }

private:
  soup::client::Client client_;
  std::optional<Connection> connection_;
};

void run(int time) {
  asio::io_context io_context;
  Io_context_runner io_runner(io_context);
  std::atomic<bool> keep_going = true;
  io_runner.set_signal_handler([&keep_going] { keep_going = false; });
  Client client(io_context);
  client.initialize();

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
