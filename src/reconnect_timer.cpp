#include "bc/soup/reconnect_timer.h"

namespace bc::soup {

Reconnect_timer::Reconnect_timer(asio::any_io_executor io_executor,
                                 Handler& handler)
    : handler_(&handler), timer_(io_executor) {}

void Reconnect_timer::start(std::chrono::seconds delay) {
  if (started_)
    return;
  started_ = true;

  try {
    timer_.expires_after(delay);
  } catch (const asio::system_error& e) {
    started_ = false;
    handler_->reconnect_timer_error(e.code(), "reconnect timer expires_after");
    return;
  }
  ++epoch_;
  timer_.async_wait([this, epoch = epoch_](asio::error_code ec) {
    if (epoch != epoch_)
      return;
    on_expiry(ec);
  });
}

void Reconnect_timer::stop() {
  if (!started_)
    return;
  started_ = false;

  try {
    timer_.cancel();
  } catch (const asio::system_error& e) {
    handler_->reconnect_timer_error(e.code(), "reconnect timer cancel");
  }
}

void Reconnect_timer::on_expiry(asio::error_code ec) {
  if (ec) {
    started_ = false;
    if (ec != asio::error::operation_aborted)
      handler_->reconnect_timer_error(ec, "reconnect timer async_wait");
    return;
  }
  if (!started_)
    return;

  started_ = false;
  handler_->reconnect_timer_expired();
}

} // namespace bc::soup
