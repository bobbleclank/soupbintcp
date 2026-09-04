#ifndef INCLUDE_BC_SOUP_SERVER_LOGIN_REJECT_H
#define INCLUDE_BC_SOUP_SERVER_LOGIN_REJECT_H

#include "bc/soup/logical_packets.h"
#include "bc/soup/server/types.h"

namespace bc::soup::server {

struct Login_reject {
  explicit Login_reject(Login_reject_reason);

  Login_reject_reason reason = Login_reject_reason::user_not_found;
  Login_rejected_packet packet;
};

} // namespace bc::soup::server

#endif
