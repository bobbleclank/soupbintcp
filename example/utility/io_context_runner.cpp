#include "io_context_runner.h"

#include <csignal>
#include <iostream>

Io_context_runner::Io_context_runner(asio::io_context& io_context)
    : io_context_(io_context) {}

Io_context_runner::~Io_context_runner() {
  if (thread_.joinable())
    stop();
}

void Io_context_runner::set_signal_handler(
    const std::function<void()>& signal_handler) {
  signal_handler_ = signal_handler;
  add_signal(SIGINT);
  add_signal(SIGTERM);
  signals_.async_wait([this](asio::error_code ec, int signal_number) {
    std::cout << "signal occurred: signal number = " << signal_number << '\n';
    if (ec) {
      std::cout << "signal occurred: error: " << ec.message() << " ("
                << ec.value() << ")\n";
      return;
    }
    signal_handler_();
  });
}

void Io_context_runner::start() {
  thread_ = std::thread([this] {
    asio::executor_work_guard work_guard(io_context_.get_executor());
    asio::io_context::count_type count = io_context_.run();
    std::cout << "number of IO context handlers executed = " << count << '\n';
  });
}

void Io_context_runner::stop() {
  io_context_.stop();
  thread_.join();
}

void Io_context_runner::add_signal(int signal_number) {
  asio::error_code ec;
  signals_.add(signal_number, ec);
  if (ec)
    std::cout << "add signal to signal set failure: " << ec.message() << " ("
              << ec.value() << ")\n";
}
