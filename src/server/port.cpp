#include "bc/soup/server/port.h"

#include "bc/soup/logical_packets.h"
#include "bc/soup/server/handler.h"
#include "bc/soup/server/tcp_connection.h"
#include "bc/soup/types.h"

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

expected<Login_accepted_packet, Login_rejected_packet>
Port::on_login_request(Tcp_connection& connection,
                       const Login_request_packet& request,
                       std::string_view session, Port_handler*& handler) {
  handler = handler_;
  connection_ = &connection;

  if (request.password != password_) {
    handler_->login_failure(Login_reject_reason::incorrect_password);
    return unexpected(
        Login_rejected_packet(Login_rejected_reason::not_authorized));
  }
  if (!request.session.empty() && request.session != session) {
    handler_->login_failure(Login_reject_reason::session_not_available);
    return unexpected(
        Login_rejected_packet(Login_rejected_reason::session_not_available));
  }

  const Login_accepted_packet response(session, 1);
  handler_->login_success(response);
  return response;
}

} // namespace bc::soup::server
