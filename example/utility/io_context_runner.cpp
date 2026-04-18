#include "io_context_runner.h"

#include <csignal>
#include <print>

Io_context_runner::Io_context_runner(asio::io_context& io_context)
    : io_context_(io_context) {
}

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
    if (ec == asio::error::operation_aborted)
      return;
    std::println("signal occurred: signal number = {}", signal_number);
    if (ec) {
      std::println("asynchronous wait against signal set failure: {} ({})",
                   ec.message(), ec.value());
      return;
    }
    if (signal_handler_)
      signal_handler_();
  });
}

void Io_context_runner::start() {
  work_guard_.emplace(asio::make_work_guard(io_context_));
  thread_ = std::thread([this] {
    const asio::io_context::count_type count = io_context_.run();
    std::println("number of IO context handlers executed = {}", count);
  });
}

void Io_context_runner::stop() {
  signals_.cancel();
  work_guard_.reset();
  if (thread_.joinable())
    thread_.join();
}

void Io_context_runner::add_signal(int signal_number) {
  asio::error_code ec;
  signals_.add(signal_number, ec);
  if (ec)
    std::println("add signal to signal set failure: {} ({})", ec.message(),
                 ec.value());
}
