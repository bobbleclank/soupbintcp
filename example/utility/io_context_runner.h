#ifndef EXAMPLE_UTILITY_IO_CONTEXT_RUNNER_H
#define EXAMPLE_UTILITY_IO_CONTEXT_RUNNER_H

#include <asio.hpp>

#include <functional>
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
  asio::io_context& io_context_;
  asio::signal_set signals_{io_context_};
  std::function<void()> signal_handler_;
  std::thread thread_;

  void add_signal(int);
};

#endif
