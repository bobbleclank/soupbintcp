#ifndef INCLUDE_BC_SOUP_CONNECTION_STATE_H
#define INCLUDE_BC_SOUP_CONNECTION_STATE_H

#include "bc/soup/types.h"

namespace bc::soup {

class Connection_state {
public:
  enum class State {
    connecting,
    connected,
    logged_in,
    disconnecting,
    disconnected
  };

  void set_state(State);

  State state() const { return state_; }
  Disconnect_reason reason() const { return reason_; }

  bool is_closing() const {
    return state_ == State::disconnecting || state_ == State::disconnected;
  }

  bool initiate_disconnect(Disconnect_reason);
  bool disconnect(Disconnect_reason);

private:
  State state_ = State::connecting;
  Disconnect_reason reason_ = Disconnect_reason::none;
};

} // namespace bc::soup

#endif
