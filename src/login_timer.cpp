#include "bc/soup/login_timer.h"

namespace bc::soup {

Login_timer::Login_timer(asio::any_io_executor io_executor, Handler& handler,
                         std::chrono::seconds timeout)
    : handler_(&handler), timer_(io_executor), timeout_(timeout) {}

void Login_timer::start() {
  if (started_)
    return;
  started_ = true;

  try {
    timer_.expires_after(timeout_);
  } catch (const asio::system_error& e) {
    handler_->login_timer_error(e.code(), "expires_after");
    return;
  }
  wait_pending_ = true;
  timer_.async_wait([this](asio::error_code ec) {
    wait_pending_ = false;
    on_expiry(ec);
    maybe_signal_stopped();
  });
}

void Login_timer::stop() {
  if (!started_)
    return;
  started_ = false;

  cancel();
  maybe_signal_stopped();
}

void Login_timer::cancel() {
  try {
    timer_.cancel();
  } catch (const asio::system_error& e) {
    handler_->login_timer_error(e.code(), "cancel");
  }
}

void Login_timer::on_expiry(asio::error_code ec) {
  if (ec) {
    if (ec != asio::error::operation_aborted)
      handler_->login_timer_error(ec, "async_wait");
    return;
  }
  if (!started_)
    return;

  handler_->login_timer_expired();
}

void Login_timer::maybe_signal_stopped() {
  if (!started_ && !stopped_signaled_ && !wait_pending_) {
    stopped_signaled_ = true;
    asio::post(timer_.get_executor(),
               [this] { handler_->login_timer_stopped(); });
  }
}

} // namespace bc::soup
