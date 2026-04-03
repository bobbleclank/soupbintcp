#include "bc/soup/server/port.h"

#include "bc/soup/server/handler.h"

namespace bc::soup::server {

Port::Port(std::string_view username, std::string_view password,
           Port_handler* handler)
    : handler_(handler), username_(username), password_(password) {
}

void Port::set_handler(Port_handler& handler) {
  handler_ = &handler;
}

bool Port::is_handler_set() const {
  return handler_ != nullptr;
}

} // namespace bc::soup::server
