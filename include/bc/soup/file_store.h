#ifndef INCLUDE_BC_SOUP_FILE_STORE_H
#define INCLUDE_BC_SOUP_FILE_STORE_H

#include "bc/soup/rw_packets.h"

#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace bc::soup {

namespace internal {

enum class Read_status {
  success,
  end_of_file,
  failure
};

enum class Write_status {
  success,
  failure
};

template <typename Status>
struct Rw_result {
  Status status = Status::success;
  size_t nbyte = 0;
};

using Read_result = Rw_result<Read_status>;
using Write_result = Rw_result<Write_status>;

template <typename Read>
Read_result read_partial_handling(int fd, void* buf, size_t nbyte,
                                  Read&& read) {
  std::span<unsigned char> s(static_cast<unsigned char*>(buf), nbyte);
  while (!s.empty()) {
    const auto n =
        std::invoke(std::forward<Read>(read), fd, s.data(), s.size());
    if (n == -1)
      return {Read_status::failure, nbyte - s.size()};
    if (n == 0)
      return {Read_status::end_of_file, nbyte - s.size()};
    s = s.last(s.size() - static_cast<size_t>(n));
  }
  return {Read_status::success, nbyte};
}

template <typename Write>
Write_result write_partial_handling(int fd, const void* buf, size_t nbyte,
                                    Write&& write) {
  std::span<const unsigned char> s(static_cast<const unsigned char*>(buf),
                                   nbyte);
  while (!s.empty()) {
    const auto n =
        std::invoke(std::forward<Write>(write), fd, s.data(), s.size());
    if (n == -1)
      return {Write_status::failure, nbyte - s.size()};
    s = s.last(s.size() - static_cast<size_t>(n));
  }
  return {Write_status::success, nbyte};
}

} // namespace internal

class Message {
public:
  Message() = default;

  explicit Message(std::size_t size) : message_(size) {}

  void* data() { return message_.data(); }

  const void* data() const { return message_.data(); }

  std::size_t size() const { return message_.size(); }

private:
  Buffer message_;
};

class File_store {
public:
  File_store() = default;
  explicit File_store(std::string_view);
  ~File_store();

  File_store(const File_store&) = delete;
  File_store& operator=(const File_store&) = delete;

  File_store(File_store&&) noexcept;
  File_store& operator=(File_store&&) noexcept;

  void set_filename(std::string_view);

  [[nodiscard]] std::error_code open();
  [[nodiscard]] std::error_code close();

  [[nodiscard]] std::error_code add(const void*, std::size_t);
  [[nodiscard]] std::error_code add(const void*, const void*);
  [[nodiscard]] std::error_code get(std::size_t, std::size_t,
                                    std::vector<Message>&);

  [[nodiscard]] std::error_code sync();

  std::size_t next_sequence_number() const;

private:
  std::string filename_;
  int fd_ = -1;
  std::vector<off_t> offsets_;

  [[nodiscard]] std::error_code set_offsets();
  [[nodiscard]] std::error_code get(Message&);
};

} // namespace bc::soup

#endif
