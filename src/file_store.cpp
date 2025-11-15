#include "bc/soup/file_store.h"

#include <cerrno>
#include <utility>

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

namespace bc::soup {
namespace {

int open(const char* path, int oflag) {
  constexpr mode_t mode = 0666;
  int fd = -1;
  // NOLINTNEXTLINE(*-avoid-do-while): Clear statement of a solution
  do
    // NOLINTNEXTLINE(*-pro-type-vararg): C-style vararg function
    fd = ::open(path, oflag, mode);
  while (fd == -1 && errno == EINTR);
  return fd;
}

int close(int fd) {
  int status = -1;
  // NOLINTNEXTLINE(*-avoid-do-while): Clear statement of a solution
  do
    status = ::close(fd);
  while (status == -1 && errno == EINTR);
  return status;
}

ssize_t read(int fd, void* buf, size_t nbyte) {
  auto* ptr = static_cast<unsigned char*>(buf);
  ssize_t total = 0;
  while (total != static_cast<ssize_t>(nbyte)) {
    ssize_t n = -1;
    // NOLINTNEXTLINE(*-avoid-do-while): Clear statement of a solution
    do
      n = ::read(fd, ptr + total, nbyte - total);
    while (n == -1 && errno == EINTR);
    if (n == -1)
      return -1;
    if (n == 0)
      return 0;
    total += n;
  }
  return total;
}

ssize_t write(int fd, const void* buf, size_t nbyte) {
  const auto* ptr = static_cast<const unsigned char*>(buf);
  ssize_t total = 0;
  while (total != static_cast<ssize_t>(nbyte)) {
    ssize_t n = -1;
    // NOLINTNEXTLINE(*-avoid-do-while): Clear statement of a solution
    do
      n = ::write(fd, ptr + total, nbyte - total);
    while (n == -1 && errno == EINTR);
    if (n == -1)
      return -1;
    total += n;
  }
  return total;
}

} // namespace

File_store::File_store(std::string_view filename) : filename_(filename) {
}

File_store::~File_store() {
  (void)close();
}

File_store::File_store(File_store&& other) noexcept
    : filename_(std::move(other.filename_)),
      fd_(other.fd_),
      offsets_(std::move(other.offsets_)) {
  other.fd_ = -1;
}

File_store& File_store::operator=(File_store&& other) noexcept {
  (void)close();
  filename_ = std::move(other.filename_);
  fd_ = other.fd_;
  offsets_ = std::move(other.offsets_);
  other.fd_ = -1;
  return *this;
}

void File_store::set_filename(std::string_view filename) {
  filename_ = filename;
}

std::error_code File_store::open() {
  fd_ = soup::open(filename_.c_str(), O_RDONLY);
  if (fd_ != -1) {
    const auto ec = set_offsets();
    (void)close();
    if (ec)
      return ec;
  }
  fd_ = soup::open(filename_.c_str(), O_RDWR | O_CREAT);
  if (fd_ == -1)
    return {errno, std::system_category()};
  return {};
}

std::error_code File_store::close() {
  if (fd_ == -1)
    return {};
  const int status = soup::close(fd_);
  fd_ = -1;
  if (status == -1)
    return {errno, std::system_category()};
  return {};
}

std::error_code File_store::add(const void* data, std::size_t size) {
  const off_t off = lseek(fd_, 0, SEEK_END);
  if (off == -1)
    return {errno, std::system_category()};
  offsets_.push_back(off);

  {
    const std::uint16_t sz = htons(size);
    const ssize_t n = soup::write(fd_, &sz, sizeof(sz));
    if (n == -1)
      return {errno, std::system_category()};
  }

  {
    const ssize_t n = soup::write(fd_, data, size);
    if (n == -1)
      return {errno, std::system_category()};
  }
  return {};
}

std::error_code File_store::add(const void* begin, const void* end) {
  const auto* b = static_cast<const std::byte*>(begin);
  const auto* e = static_cast<const std::byte*>(end);
  const auto size = static_cast<std::size_t>(e - b);
  return add(begin, size);
}

std::error_code File_store::get(std::size_t first, std::size_t last,
                                std::vector<Message>& messages) {
  if (first < 1 || first > offsets_.size())
    return {EINVAL, std::system_category()};
  if (last < first || last > offsets_.size())
    return {EINVAL, std::system_category()};

  const off_t off = lseek(fd_, offsets_[first - 1], SEEK_SET);
  if (off == -1)
    return {errno, std::system_category()};

  for (std::size_t i = first; i <= last; ++i) {
    Message message;
    const auto ec = get(message);
    if (ec)
      return ec;
    messages.push_back(std::move(message));
  }
  return {};
}

std::error_code File_store::sync() {
  const int status = fsync(fd_);
  if (status == -1)
    return {errno, std::system_category()};
  return {};
}

std::size_t File_store::next_sequence_number() const {
  return offsets_.size() + 1;
}

std::error_code File_store::set_offsets() {
  off_t off = 0;
  while (true) {
    std::uint16_t sz = 0;
    const ssize_t n = soup::read(fd_, &sz, sizeof(sz));
    if (n == -1)
      return {errno, std::system_category()};
    if (n == 0)
      break;
    const std::uint16_t size = ntohs(sz);

    offsets_.push_back(off);
    off = lseek(fd_, size, SEEK_CUR);
    if (off == -1)
      return {errno, std::system_category()};
  }
  return {};
}

std::error_code File_store::get(Message& message) {
  std::uint16_t sz = 0;
  {
    const ssize_t n = soup::read(fd_, &sz, sizeof(sz));
    if (n == -1)
      return {errno, std::system_category()};
    if (n == 0)
      return {EIO, std::system_category()};
  }
  const std::uint16_t size = ntohs(sz);

  message = Message(size);
  {
    const ssize_t n = soup::read(fd_, message.data(), message.size());
    if (n == -1)
      return {errno, std::system_category()};
    if (n == 0)
      return {EIO, std::system_category()};
  }
  return {};
}

} // namespace bc::soup
