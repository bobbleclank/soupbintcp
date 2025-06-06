#ifndef INCLUDE_BC_SOUP_SOCKET_H
#define INCLUDE_BC_SOUP_SOCKET_H

#include "bc/soup/rw_packets.h"
#include "bc/soup/types.h"

#include <asio.hpp>

#include <cstddef>
#include <list>

namespace bc::soup {

class Socket {
public:
  class Handler {
  public:
    virtual void connect_failure(asio::error_code) = 0;
    virtual void connect_success() = 0;

    virtual void read_failure(asio::error_code) = 0;
    virtual void read_failure(Packet_error) = 0;
    virtual void read_completed(const Read_packet&) = 0;
    virtual void end_of_file() = 0;

    virtual void write_failure(asio::error_code) = 0;
    virtual void write_completed(const Write_packet&) = 0;
    virtual void write_buffer_empty() = 0;

  protected:
    Handler() = default;
    ~Handler() = default;

    Handler(const Handler&) = default;
    Handler& operator=(const Handler&) = default;

    Handler(Handler&&) = default;
    Handler& operator=(Handler&&) = default;
  };

  explicit Socket(asio::any_io_executor);
  Socket(asio::any_io_executor, Handler&);
  explicit Socket(asio::ip::tcp::socket&&);
  Socket(asio::ip::tcp::socket&&, Handler&);
  ~Socket() = default;

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  Socket(Socket&&) = default;
  Socket& operator=(Socket&&) noexcept = default;

  void set_handler(Handler&);
  void set_write_packets_limit(std::size_t);

  [[nodiscard]] asio::error_code open();
  void shutdown(asio::error_code* = nullptr);
  void close(asio::error_code* = nullptr);

  [[nodiscard]] asio::error_code set_no_delay();

  void async_connect(const asio::ip::tcp::endpoint&);

  void async_read();
  Write_error async_write(Write_packet&&);

  asio::ip::tcp::endpoint local_endpoint(asio::error_code* = nullptr) const;
  asio::ip::tcp::endpoint remote_endpoint(asio::error_code* = nullptr) const;

  asio::ip::tcp::socket::executor_type get_executor();

private:
  Handler* handler_ = nullptr;
  asio::ip::tcp::socket socket_;
  Read_packet read_packet_;
  std::list<Write_packet> write_packets_;
  // NOLINTNEXTLINE(*-avoid-magic-numbers): Default value
  std::size_t write_packets_limit_ = 100;
  bool write_buffer_was_full_ = false;

  void read_header();
  void header_received(asio::error_code, std::size_t);
  void read_payload();
  void payload_received(asio::error_code, std::size_t);
  void write_packet();
  void packet_sent(asio::error_code, std::size_t);
};

} // namespace bc::soup

#endif
