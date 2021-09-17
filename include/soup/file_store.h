#ifndef INCLUDE_FILE_STORE_H
#define INCLUDE_FILE_STORE_H

#include "soup/rw_packets.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <sys/types.h>

namespace soup {

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

} // namespace soup

#endif
