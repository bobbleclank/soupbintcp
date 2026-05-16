#include "bc/soup/connection_state.h"

namespace bc::soup {

void Connection_state::set_state(State state) {
  state_ = state;
}

bool Connection_state::initiate_disconnect(Disconnect_reason pending_reason) {
  if (is_closing())
    return false;
  state_ = State::disconnecting;
  reason_ = pending_reason;
  return true;
}

bool Connection_state::disconnect(Disconnect_reason observed_reason) {
  if (state_ == State::disconnected)
    return false;
  state_ = State::disconnected;
  if (reason_ == Disconnect_reason::none)
    reason_ = observed_reason;
  return true;
}

} // namespace bc::soup
