#include "bc/soup/socket_acceptor.h"

#include <utility>

namespace bc::soup {

Socket_acceptor::Socket_acceptor(asio::any_io_executor io_executor)
    : acceptor_(io_executor) {
}

Socket_acceptor::Socket_acceptor(asio::any_io_executor io_executor,
                                 Handler& handler)
    : handler_(&handler), acceptor_(io_executor) {
}

void Socket_acceptor::set_handler(Handler& handler) {
  handler_ = &handler;
}

asio::error_code Socket_acceptor::open() {
  asio::error_code ec;
  acceptor_.open(asio::ip::tcp::v4(), ec);
  return ec;
}

asio::error_code
Socket_acceptor::bind(const asio::ip::tcp::endpoint& endpoint) {
  asio::error_code ec;
  acceptor_.bind(endpoint, ec);
  return ec;
}

asio::error_code Socket_acceptor::listen() {
  asio::error_code ec;
  acceptor_.listen(asio::socket_base::max_listen_connections, ec);
  return ec;
}

void Socket_acceptor::close(asio::error_code* error) {
  asio::error_code ec;
  acceptor_.close(ec);
  if (error)
    *error = ec;
}

asio::error_code Socket_acceptor::set_reuse_address() {
  asio::ip::tcp::acceptor::reuse_address option(true);
  asio::error_code ec;
  acceptor_.set_option(option, ec);
  return ec;
}

asio::error_code Socket_acceptor::set_no_delay() {
  asio::ip::tcp::no_delay option(true);
  asio::error_code ec;
  acceptor_.set_option(option, ec);
  return ec;
}

void Socket_acceptor::async_accept() {
  socket_.emplace(acceptor_.get_executor());
  acceptor_.async_accept(*socket_, [this](asio::error_code ec) {
    if (ec == asio::error::operation_aborted) {
      return;
    }
    if (ec) {
      handler_->accept_failure(ec);
      return;
    }
    handler_->accept_success(std::move(*socket_));
  });
}

asio::ip::tcp::endpoint
Socket_acceptor::local_endpoint(asio::error_code* error) const {
  asio::error_code ec;
  auto endpoint = acceptor_.local_endpoint(ec);
  if (error)
    *error = ec;
  return endpoint;
}

asio::ip::tcp::acceptor::executor_type Socket_acceptor::get_executor() {
  return acceptor_.get_executor();
}

} // namespace bc::soup
