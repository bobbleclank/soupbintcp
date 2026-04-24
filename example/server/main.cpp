#include "bc/soup/expected.h"
#include "bc/soup/server/acceptor.h"
#include "bc/soup/server/handler.h"
#include "bc/soup/server/port.h"
#include "bc/soup/server/server.h"
#include "bc/soup/types.h"
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

class Port final : public soup::server::Port_handler {
public:
  explicit Port(soup::server::Port* port) : port_(port) {
    port_->set_handler(*this);
  }

private:
  soup::server::Port* port_ = nullptr;
};

class Acceptor final : public soup::server::Acceptor_handler {
public:
  explicit Acceptor(soup::server::Acceptor* acceptor) : acceptor_(acceptor) {
    acceptor_->set_handler(*this);
  }

  void initialize() {
    const auto result = acceptor_->add_port("", "");
    if (!result)
      throw std::system_error(result.error(), "add port");
    port_.emplace(*result);
  }

  void listening_setup_failure(asio::error_code ec,
                               const char* phase) override {
    std::println("listening setup failure: error = {}:{} {}, phase = {}",
                 ec.category().name(), ec.value(), ec.message(), phase);
  }

  void listening_setup_success(const asio::ip::tcp::endpoint& ep) override {
    std::println("listening setup success: endpoint = {}:{}",
                 ep.address().to_string(), ep.port());
  }

  void accept_failure(asio::error_code ec) override {
    std::println("accept failure: error = {}:{} {}", ec.category().name(),
                 ec.value(), ec.message());
  }

  void accept_success(const asio::ip::tcp::endpoint& local_ep,
                      const asio::ip::tcp::endpoint& remote_ep) override {
    std::println(
        "accept success: local endpoint = {}:{}, remote endpoint = {}:{}",
        local_ep.address().to_string(), local_ep.port(),
        remote_ep.address().to_string(), remote_ep.port());
  }

  void disconnect(soup::Disconnect_reason reason) override {
    std::println("disconnect: reason = {}", to_string(reason));
  }

private:
  soup::server::Acceptor* acceptor_ = nullptr;
  std::optional<Port> port_;
};

class Server {
public:
  explicit Server(asio::io_context& io_context)
      : server_(io_context.get_executor()) {}

  void initialize() {
    const unsigned short port = 5050;
    const asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), port);
    const auto result = server_.add_acceptor(ep);
    if (!result)
      throw std::system_error(result.error(), "add acceptor");
    acceptor_.emplace(*result);
    acceptor_->initialize();
  }

  void start() {
    if (const auto ec = server_.start())
      throw std::system_error(ec, "server start");
  }

  void stop() { server_.stop(); }

private:
  soup::server::Server server_;
  std::optional<Acceptor> acceptor_;
};

void run(int time) {
  asio::io_context io_context;
  Io_context_runner io_runner(io_context);
  std::atomic<bool> keep_going = true;
  io_runner.set_signal_handler([&keep_going] { keep_going = false; });
  Server server(io_context);
  server.initialize();

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
  std::print("usage: bc_soup_server [options]\n"
             "options:\n"
             "  -h  help\n"
             "  -t  running time (seconds) [30]\n"
             "  -v  version\n");
}

void display_version() {
  std::println("version {}.{}", bc_soup_VERSION_MAJOR, bc_soup_VERSION_MINOR);
}

int main(int argc, char** argv) {
  constexpr int default_time = 30;
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
