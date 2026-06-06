#ifndef INCLUDE_BC_SOUP_LOGIN_TIMER_H
#define INCLUDE_BC_SOUP_LOGIN_TIMER_H

#include <asio.hpp>

#include <chrono>

namespace bc::soup {

class Login_timer {
public:
  class Handler {
  public:
    virtual void login_timer_error(const asio::system_error&) = 0;
    virtual void login_timer_expired() = 0;
    virtual void login_timer_stopped() = 0;

  protected:
    Handler() = default;
    ~Handler() = default;

    Handler(const Handler&) = default;
    Handler& operator=(const Handler&) = default;

    Handler(Handler&&) = default;
    Handler& operator=(Handler&&) = default;
  };

  Login_timer(asio::any_io_executor, Handler&, std::chrono::seconds);

  void start();
  void stop();

private:
  Handler* handler_ = nullptr;
  asio::steady_timer timer_;
  std::chrono::seconds timeout_ = std::chrono::seconds::zero();
  bool started_ = false;
  bool wait_pending_ = false;
  bool stopped_signaled_ = false;

  void cancel();
  void on_expiry(asio::error_code);
  void maybe_signal_stopped();
};

} // namespace bc::soup

#endif
