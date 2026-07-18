#ifndef INCLUDE_BC_SOUP_LOGIN_REJECT_H
#define INCLUDE_BC_SOUP_LOGIN_REJECT_H

#include "bc/soup/logical_packets.h"
#include "bc/soup/types.h"

namespace bc::soup {

struct Login_reject {
  Login_reject(Login_reject_reason reason_,
               Login_rejected_packet::Reason packet_reason)
      : reason(reason_), packet(packet_reason) {}

  Login_reject_reason reason = Login_reject_reason::none;
  Login_rejected_packet packet;
};

} // namespace bc::soup

#endif
