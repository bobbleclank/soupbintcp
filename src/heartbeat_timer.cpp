#include "bc/soup/heartbeat_timer.h"

#include "bc/soup/constants.h"

namespace bc::soup {

Heartbeat_timer::Heartbeat_timer(asio::steady_timer& timer, Handler& handler,
                                 std::chrono::seconds timeout)
    : handler_(&handler), timer_(&timer), timeout_(timeout) {
}

void Heartbeat_timer::start() {
  try {
    timer_->expires_after(heartbeat_period);
  } catch (const asio::system_error& e) {
    handler_->heartbeat_timer_error(e);
    return;
  }
  timer_->async_wait([this](asio::error_code ec) { on_expiry(ec); });
}

void Heartbeat_timer::stop() {
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

  if (receive_count_ == 0) {
    no_receive_period_ += heartbeat_period;
    if (no_receive_period_ >= timeout_) {
      handler_->timed_out();
      return;
    }
  } else {
    no_receive_period_ = std::chrono::seconds::zero();
    receive_count_ = 0;
  }

  if (send_count_ == 0)
    handler_->heartbeat_required();
  else
    send_count_ = 0;

  start();
}

} // namespace bc::soup
