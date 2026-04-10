#include "bc/soup/heartbeat_timer.h"

#include "bc/soup/constants.h"

namespace bc::soup {

Heartbeat_timer::Heartbeat_timer(asio::steady_timer& timer, Handler& handler,
                                 std::chrono::seconds timeout)
    : handler_(&handler), timer_(&timer), timeout_(timeout) {
}

void Heartbeat_timer::start() {
  if (started_)
    return;
  started_ = true;

  timer_->expires_at(std::chrono::steady_clock::now());
  schedule();
}

void Heartbeat_timer::stop() {
  if (!started_)
    return;
  started_ = false;

  cancel();
}

void Heartbeat_timer::schedule() {
  try {
    timer_->expires_at(timer_->expiry() + heartbeat_period);
  } catch (const asio::system_error& e) {
    handler_->heartbeat_timer_error(e);
    return;
  }
  timer_->async_wait([this](asio::error_code ec) { on_expiry(ec); });
}

void Heartbeat_timer::cancel() {
  try {
    timer_->cancel();
  } catch (const asio::system_error& e) {
    handler_->heartbeat_timer_error(e);
  }
}

void Heartbeat_timer::on_expiry(asio::error_code ec) {
  if (ec == asio::error::operation_aborted) {
    return;
  }
  if (ec) {
    handler_->heartbeat_timer_error({ec, "async_wait"});
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

  schedule();
}

} // namespace bc::soup
