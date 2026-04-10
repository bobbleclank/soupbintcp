#ifndef INCLUDE_BC_SOUP_HEARTBEAT_TIMER_H
#define INCLUDE_BC_SOUP_HEARTBEAT_TIMER_H

#include <asio.hpp>

#include <chrono>
#include <cstdint>

namespace bc::soup {

class Heartbeat_timer {
public:
  class Handler {
  public:
    virtual void heartbeat_timer_error(const asio::system_error&) = 0;
    virtual void heartbeat_send_due() = 0;
    virtual void heartbeat_receive_timeout() = 0;

  protected:
    Handler() = default;
    ~Handler() = default;

    Handler(const Handler&) = default;
    Handler& operator=(const Handler&) = default;

    Handler(Handler&&) = default;
    Handler& operator=(Handler&&) = default;
  };

  Heartbeat_timer(asio::steady_timer&, Handler&, std::chrono::seconds);

  void start();
  void stop();

  void increment_receive_count() { ++receive_count_; }
  void increment_send_count() { ++send_count_; }

private:
  Handler* handler_ = nullptr;
  asio::steady_timer* timer_ = nullptr;
  std::chrono::seconds timeout_ = std::chrono::seconds::zero();
  std::chrono::seconds no_receive_period_ = std::chrono::seconds::zero();
  std::uint32_t receive_count_ = 0;
  std::uint32_t send_count_ = 0;
  bool started_ = false;

  void schedule();
  void cancel();
  void on_expiry(asio::error_code);
};

} // namespace bc::soup

#endif
