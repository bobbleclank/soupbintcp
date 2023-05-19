#ifndef INCLUDE_BC_SOUP_SOCKET_H
#define INCLUDE_BC_SOUP_SOCKET_H

#include "bc/soup/rw_packets.h"
#include "bc/soup/types.h"

#include <boost/asio.hpp>

#include <cstddef>
#include <list>

namespace bc::soup {

class Socket {
public:
  class Handler {
  public:
    virtual void connect_failure(boost::system::error_code) = 0;
    virtual void connect_success() = 0;

    virtual void read_failure(boost::system::error_code) = 0;
    virtual void read_failure(Packet_error) = 0;
    virtual void read_completed(const Read_packet&) = 0;
    virtual void end_of_file() = 0;

    virtual void write_failure(boost::system::error_code) = 0;
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

  explicit Socket(boost::asio::any_io_executor);
  Socket(boost::asio::any_io_executor, Handler&);
  explicit Socket(boost::asio::ip::tcp::socket&&);
  Socket(boost::asio::ip::tcp::socket&&, Handler&);
  ~Socket() = default;

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  Socket(Socket&&) = default;
  Socket& operator=(Socket&&) = default;

  void set_handler(Handler&);
  void set_write_packets_limit(std::size_t);

  [[nodiscard]] boost::system::error_code open();
  void shutdown(boost::system::error_code* = nullptr);
  void close(boost::system::error_code* = nullptr);

  [[nodiscard]] boost::system::error_code set_no_delay();

  void async_connect(const boost::asio::ip::tcp::endpoint&);

  void async_read();
  Write_error async_write(Write_packet&&);

  boost::asio::ip::tcp::endpoint local_endpoint(boost::system::error_code* = nullptr) const;
  boost::asio::ip::tcp::endpoint remote_endpoint(boost::system::error_code* = nullptr) const;

  boost::asio::ip::tcp::socket::executor_type get_executor();

private:
  Handler* handler_ = nullptr;
  boost::asio::ip::tcp::socket socket_;
  Read_packet read_packet_;
  std::list<Write_packet> write_packets_;
  std::size_t write_packets_limit_ = 100;
  bool write_buffer_was_full_ = false;

  void read_header();
  void header_received(boost::system::error_code, std::size_t);
  void read_payload();
  void payload_received(boost::system::error_code, std::size_t);
  void write_packet();
  void packet_sent(boost::system::error_code, std::size_t);
};

} // namespace bc::soup

#endif
