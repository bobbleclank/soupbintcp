#include "bc/soup/heartbeat_timer.h"

#include "bc/soup/constants.h"

namespace bc::soup {

Heartbeat_timer::Heartbeat_timer(asio::any_io_executor io_executor,
                                 Handler& handler, std::chrono::seconds timeout)
    : handler_(&handler), timer_(io_executor), timeout_(timeout) {}

void Heartbeat_timer::start() {
  if (started_)
    return;
  started_ = true;

  try {
    timer_.expires_at(std::chrono::steady_clock::now());
  } catch (const asio::system_error& e) {
    handler_->heartbeat_timer_error(e.code(), "expires_at");
    return;
  }
  schedule();
}

void Heartbeat_timer::stop() {
  if (!started_)
    return;
  started_ = false;

  cancel();
  maybe_signal_stopped();
}

void Heartbeat_timer::schedule() {
  try {
    timer_.expires_at(timer_.expiry() + heartbeat_period);
  } catch (const asio::system_error& e) {
    handler_->heartbeat_timer_error(e.code(), "expires_at");
    return;
  }
  wait_pending_ = true;
  timer_.async_wait([this](asio::error_code ec) {
    wait_pending_ = false;
    on_expiry(ec);
    maybe_signal_stopped();
  });
}

void Heartbeat_timer::cancel() {
  try {
    timer_.cancel();
  } catch (const asio::system_error& e) {
    handler_->heartbeat_timer_error(e.code(), "cancel");
  }
}

void Heartbeat_timer::on_expiry(asio::error_code ec) {
  if (ec) {
    if (ec != asio::error::operation_aborted)
      handler_->heartbeat_timer_error(ec, "async_wait");
    return;
  }
  if (!started_)
    return;

  if (receive_count_ == 0) {
    no_receive_period_ += heartbeat_period;
    if (no_receive_period_ >= timeout_) {
      handler_->heartbeat_receive_timeout();
      return;
    }
  } else {
    no_receive_period_ = std::chrono::seconds::zero();
    receive_count_ = 0;
  }

  if (send_count_ == 0)
    handler_->heartbeat_send_due();
  else
    send_count_ = 0;

  if (!started_)
    return;
  schedule();
}

void Heartbeat_timer::maybe_signal_stopped() {
  if (!started_ && !stopped_signaled_ && !wait_pending_) {
    stopped_signaled_ = true;
    asio::post(timer_.get_executor(),
               [this] { handler_->heartbeat_timer_stopped(); });
  }
}

} // namespace bc::soup
