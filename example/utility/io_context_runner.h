#ifndef EXAMPLE_UTILITY_IO_CONTEXT_RUNNER_H
#define EXAMPLE_UTILITY_IO_CONTEXT_RUNNER_H

#include <asio.hpp>

#include <functional>
#include <optional>
#include <thread>

class Io_context_runner {
public:
  explicit Io_context_runner(asio::io_context&);
  ~Io_context_runner();

  Io_context_runner(const Io_context_runner&) = delete;
  Io_context_runner(Io_context_runner&&) = delete;

  Io_context_runner& operator=(const Io_context_runner&) = delete;
  Io_context_runner& operator=(Io_context_runner&&) = delete;

  void set_signal_handler(const std::function<void()>&);

  void start();
  void stop();

private:
  using executor = asio::io_context::executor_type;
  using work_guard = asio::executor_work_guard<executor>;

  asio::io_context& io_context_;
  std::optional<work_guard> work_guard_;
  asio::signal_set signals_{io_context_};
  std::function<void()> signal_handler_;
  std::thread thread_;

  void add_signal(int);
};

#endif
