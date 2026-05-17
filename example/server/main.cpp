#include "bc/soup/expected.h"
#include "bc/soup/logical_packets.h"
#include "bc/soup/server/acceptor.h"
#include "bc/soup/server/handler.h"
#include "bc/soup/server/port.h"
#include "bc/soup/server/server.h"
#include "bc/soup/types.h"
#include "bc_soup_config.h"
#include "io_context_runner.h"
#include "option_error.h"

#include <asio.hpp>

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <print>
#include <string>
#include <string_view>
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

  void login_failure(soup::Login_reject_reason reason) override {
    std::println("login failure: reason = {}", to_string(reason));
  }

  void login_success(const soup::Login_accepted_packet& p) override {
    std::println("login success: session = {}, next sequence number = {}",
                 p.session, p.next_sequence_number);
  }

  void unsequenced_data(const void* data, std::size_t size) override {
    const std::string_view message(static_cast<const char*>(data), size);
    std::println("unsequenced data: data = {}, size = {}", message, size);
  }

  void disconnect(soup::Disconnect_reason reason) override {
    std::println("disconnect: reason = {}", to_string(reason));
  }

  void send_message() {
    static constexpr std::string_view message = "hello client";
    const auto error = port_->send_message(message.data(), message.size());
    if (error != soup::Write_error::none)
      std::println("send message: error = {}", to_string(error));
  }

private:
  soup::server::Port* port_ = nullptr;
};

class Acceptor final : public soup::server::Acceptor_handler {
public:
  explicit Acceptor(soup::server::Acceptor* acceptor) : acceptor_(acceptor) {
    acceptor_->set_handler(*this);
  }

  void initialize(std::string_view username, std::string_view password) {
    const auto result = acceptor_->add_port(username, password);
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

  void login_request(const soup::Login_request_packet& p) override {
    std::println("login request: username = {}, password = {}, session = {}, "
                 "next sequence number = {}",
                 p.username, p.password, p.session, p.next_sequence_number);
  }

  void login_failure(soup::Login_reject_reason reason) override {
    std::println("login failure: reason = {}", to_string(reason));
  }

  void disconnect(soup::Disconnect_reason reason) override {
    std::println("disconnect: reason = {}", to_string(reason));
  }

  void send_message() { port_->send_message(); }

private:
  soup::server::Acceptor* acceptor_ = nullptr;
  std::optional<Port> port_;
};

class Server {
public:
  explicit Server(asio::io_context& io_context)
      : server_(io_context.get_executor()) {}

  void initialize(std::string_view username, std::string_view password,
                  std::string_view session) {
    if (const auto ec = server_.set_session(session))
      throw std::system_error(ec, "set session");
    const unsigned short port = 5050;
    const asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), port);
    const auto result = server_.add_acceptor(ep);
    if (!result)
      throw std::system_error(result.error(), "add acceptor");
    acceptor_.emplace(*result);
    acceptor_->initialize(username, password);
  }

  void start() {
    if (const auto ec = server_.start())
      throw std::system_error(ec, "server start");
  }

  void send_message() { acceptor_->send_message(); }

  void end_session() { server_.end_session(); }

  void stop() { server_.stop(); }

private:
  soup::server::Server server_;
  std::optional<Acceptor> acceptor_;
};

void run(std::string_view username, std::string_view password,
         std::string_view session) {
  asio::io_context io_context;
  Io_context_runner io_runner(io_context);
  std::atomic<bool> keep_going = true;
  io_runner.set_signal_handler([&keep_going] { keep_going = false; });
  Server server(io_context);
  server.initialize(username, password, session);

  io_runner.start();
  std::this_thread::sleep_for(1s);
  server.start();

  std::println("press enter to send a message, q to quit");
  std::string line;
  while (keep_going && std::getline(std::cin, line)) {
    if (line == "q")
      break;
    asio::post(io_context, [&server] { server.send_message(); });
  }

  server.end_session();
  std::this_thread::sleep_for(1s);
  server.stop();
  std::this_thread::sleep_for(1s);
  io_runner.stop();
}

void display_usage() {
  std::print("usage: bc_soup_server [options]\n"
             "options:\n"
             "  -h  help\n"
             "  -p  password [pass]\n"
             "  -s  session [sess]\n"
             "  -u  username [user]\n"
             "  -v  version\n");
}

void display_version() {
  std::println("version {}.{}", bc_soup_VERSION_MAJOR, bc_soup_VERSION_MINOR);
}

int main(int argc, char** argv) {
  const char* username = "user";
  const char* password = "pass";
  const char* session = "sess";

  try {
    int opt = 0;
    while ((opt = getopt(argc, argv, ":hp:s:u:v")) != -1) {
      switch (opt) {
      case 'h':
        display_usage();
        return EXIT_SUCCESS;
      case 'p':
        password = optarg;
        break;
      case 's':
        session = optarg;
        break;
      case 'u':
        username = optarg;
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
    run(username, password, session);
  } catch (const std::system_error& e) {
    std::println("system error: {}:{} {}", e.code().category().name(),
                 e.code().value(), e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
