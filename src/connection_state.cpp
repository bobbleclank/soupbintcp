#include "bc/soup/connection_state.h"

namespace bc::soup {

void Connection_state::set_state(State state) {
  state_ = state;
}

bool Connection_state::initiate_disconnect(Disconnect_reason reason) {
  if (is_closing())
    return false;
  state_ = State::disconnecting;
  pending_reason_ = reason;
  return true;
}

Disconnect_reason
Connection_state::terminate(Disconnect_reason observed_reason) {
  if (state_ == State::disconnected)
    return Disconnect_reason::none;
  state_ = State::disconnected;
  const auto reason = (pending_reason_ == Disconnect_reason::none)
                          ? observed_reason
                          : pending_reason_;
  pending_reason_ = Disconnect_reason::none;
  return reason;
}

} // namespace bc::soup
