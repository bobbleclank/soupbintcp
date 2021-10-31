#ifndef INCLUDE_BC_SOUP_SOCKET_ACCEPTOR_H
#define INCLUDE_BC_SOUP_SOCKET_ACCEPTOR_H

#include <asio.hpp>

#include <optional>

namespace bc::soup {

class Socket_acceptor {
public:
  class Handler {
  public:
    virtual void accept_failure(asio::error_code) = 0;
    virtual void accept_success(asio::ip::tcp::socket&&) = 0;

  protected:
    Handler() = default;
    ~Handler() = default;

    Handler(const Handler&) = default;
    Handler& operator=(const Handler&) = default;

    Handler(Handler&&) = default;
    Handler& operator=(Handler&&) = default;
  };

  explicit Socket_acceptor(asio::any_io_executor);
  Socket_acceptor(asio::any_io_executor, Handler&);
  ~Socket_acceptor() = default;

  Socket_acceptor(const Socket_acceptor&) = delete;
  Socket_acceptor& operator=(const Socket_acceptor&) = delete;

  Socket_acceptor(Socket_acceptor&&) = default;
  Socket_acceptor& operator=(Socket_acceptor&&) = default;

  void set_handler(Handler&);

  [[nodiscard]] asio::error_code open();
  [[nodiscard]] asio::error_code bind(const asio::ip::tcp::endpoint&);
  [[nodiscard]] asio::error_code listen();
  void close(asio::error_code* = nullptr);

  [[nodiscard]] asio::error_code set_reuse_address();
  [[nodiscard]] asio::error_code set_no_delay();

  void async_accept();

  asio::ip::tcp::endpoint local_endpoint(asio::error_code* = nullptr) const;

  asio::ip::tcp::acceptor::executor_type get_executor();

private:
  Handler* handler_ = nullptr;
  asio::ip::tcp::acceptor acceptor_;
  std::optional<asio::ip::tcp::socket> socket_;
};

} // namespace bc::soup

#endif
