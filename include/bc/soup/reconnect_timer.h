#ifndef INCLUDE_BC_SOUP_RECONNECT_TIMER_H
#define INCLUDE_BC_SOUP_RECONNECT_TIMER_H

#include <asio.hpp>

#include <chrono>
#include <cstdint>
#include <string_view>

namespace bc::soup {

class Reconnect_timer {
public:
  class Handler {
  public:
    virtual void reconnect_timer_error(asio::error_code, std::string_view) = 0;
    virtual void reconnect_timer_expired() = 0;

  protected:
    Handler() = default;
    ~Handler() = default;

    Handler(const Handler&) = default;
    Handler& operator=(const Handler&) = default;

    Handler(Handler&&) = default;
    Handler& operator=(Handler&&) = default;
  };

  Reconnect_timer(asio::any_io_executor, Handler&);

  void start(std::chrono::seconds);
  void stop();

  bool started() const { return started_; }

private:
  Handler* handler_ = nullptr;
  asio::steady_timer timer_;
  bool started_ = false;
  std::uint32_t epoch_ = 0;

  void cancel();
  void on_expiry(asio::error_code);
};

} // namespace bc::soup

#endif
