#include "bc/soup/socket_acceptor.h"

#include <utility>

namespace bc::soup {

Socket_acceptor::Socket_acceptor(boost::asio::any_io_executor io_executor)
    : acceptor_(io_executor) {}

Socket_acceptor::Socket_acceptor(boost::asio::any_io_executor io_executor,
                                 Handler& handler)
    : handler_(&handler), acceptor_(io_executor) {}

void Socket_acceptor::set_handler(Handler& handler) { handler_ = &handler; }

boost::system::error_code Socket_acceptor::open() {
  boost::system::error_code ec;
  acceptor_.open(boost::asio::ip::tcp::v4(), ec);
  return ec;
}

boost::system::error_code Socket_acceptor::bind(const boost::asio::ip::tcp::endpoint& endpoint) {
  boost::system::error_code ec;
  acceptor_.bind(endpoint, ec);
  return ec;
}

boost::system::error_code Socket_acceptor::listen() {
  boost::system::error_code ec;
  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
  return ec;
}

void Socket_acceptor::close(boost::system::error_code* error) {
  boost::system::error_code ec;
  acceptor_.close(ec);
  if (error)
    *error = ec;
}

boost::system::error_code Socket_acceptor::set_reuse_address() {
  boost::asio::ip::tcp::acceptor::reuse_address option(true);
  boost::system::error_code ec;
  acceptor_.set_option(option, ec);
  return ec;
}

boost::system::error_code Socket_acceptor::set_no_delay() {
  boost::asio::ip::tcp::no_delay option(true);
  boost::system::error_code ec;
  acceptor_.set_option(option, ec);
  return ec;
}

void Socket_acceptor::async_accept() {
  socket_.emplace(acceptor_.get_executor());
  acceptor_.async_accept(*socket_, [this](boost::system::error_code ec) {
    if (ec == boost::asio::error::operation_aborted) {
      return;
    }
    if (ec) {
      handler_->accept_failure(ec);
      return;
    }
    handler_->accept_success(std::move(*socket_));
  });
}

boost::asio::ip::tcp::endpoint
Socket_acceptor::local_endpoint(boost::system::error_code* error) const {
  boost::system::error_code ec;
  auto endpoint = acceptor_.local_endpoint(ec);
  if (error)
    *error = ec;
  return endpoint;
}

boost::asio::ip::tcp::acceptor::executor_type Socket_acceptor::get_executor() {
  return acceptor_.get_executor();
}

} // namespace bc::soup
