#ifndef INCLUDE_EXP_EXPECTED_H
#define INCLUDE_EXP_EXPECTED_H

#include <exception>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace exp {

struct unexpect_t {
  explicit unexpect_t() = default;
};

inline constexpr unexpect_t unexpect{};

template <class E> class bad_expected_access : public std::exception {
public:
  explicit bad_expected_access(E val) : val_(std::move(val)) {}

  virtual const char* what() const noexcept override {
    return "bad expected access";
  }

  const E& error() const& { return val_; }
  E& error() & { return val_; }
  const E&& error() const&& { return std::move(val_); }
  E&& error() && { return std::move(val_); }

private:
  E val_;
};

template <class E> class unexpected {
public:
  static_assert(!std::is_same_v<E, void>);
  static_assert(!std::is_reference_v<E>);

  unexpected() = delete;

  unexpected(const unexpected&) = default;
  unexpected(unexpected&&) = default;

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  explicit unexpected(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...) {}

  ~unexpected() = default;

  unexpected& operator=(const unexpected&) = default;
  unexpected& operator=(unexpected&&) = default;

  const E& value() const& { return val_; }
  E& value() & { return val_; }
  const E&& value() const&& { return std::move(val_); }
  E&& value() && { return std::move(val_); }

private:
  E val_;
};

template <class E1, class E2>
bool operator==(const unexpected<E1>& x, const unexpected<E2>& y) {
  return x.value() == y.value();
}

template <class E1, class E2>
bool operator!=(const unexpected<E1>& x, const unexpected<E2>& y) {
  return x.value() != y.value();
}

template <class T, class E> class expected {
public:
  static_assert(!std::is_same_v<T, void>);
  static_assert(!std::is_same_v<T, std::remove_cv_t<unexpected<E>>>);
  static_assert(!std::is_same_v<T, std::remove_cv_t<std::in_place_t>>);
  static_assert(!std::is_same_v<T, std::remove_cv_t<unexpect_t>>);
  static_assert(!std::is_reference_v<T>);

  static_assert(!std::is_same_v<E, void>);
  static_assert(!std::is_reference_v<E>);

  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  expected() : val_(std::in_place) {}

  expected(const expected&) = default;
  expected(expected&&) = default;

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<T, Args&&...>>* = nullptr>
  explicit expected(std::in_place_t, Args&&... args)
      : val_(std::in_place, std::forward<Args>(args)...) {}

  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  explicit expected(unexpect_t, Args&&... args)
      : unexpect_(std::in_place, std::in_place, std::forward<Args>(args)...) {}

  ~expected() = default;

  expected& operator=(const expected&) = default;
  expected& operator=(expected&&) = default;

  const T* operator->() const { return std::addressof(*val_); }
  T* operator->() { return std::addressof(*val_); }

  const T& operator*() const& { return *val_; }
  T& operator*() & { return *val_; }
  const T&& operator*() const&& { return std::move(*val_); }
  T&& operator*() && { return std::move(*val_); }

  explicit operator bool() const { return val_.has_value(); }
  bool has_value() const { return val_.has_value(); }

  const T& value() const& {
    if (!val_.has_value())
      throw bad_expected_access(unexpect_->value());
    return *val_;
  }

  T& value() & {
    if (!val_.has_value())
      throw bad_expected_access(unexpect_->value());
    return *val_;
  }

  const T&& value() const&& {
    if (!val_.has_value())
      throw bad_expected_access(std::move(unexpect_->value()));
    return std::move(*val_);
  }

  T&& value() && {
    if (!val_.has_value())
      throw bad_expected_access(std::move(unexpect_->value()));
    return std::move(*val_);
  }

  const E& error() const& { return unexpect_->value(); }
  E& error() & { return unexpect_->value(); }
  const E&& error() const&& { return std::move(unexpect_->value()); }
  E&& error() && { return std::move(unexpect_->value()); }

private:
  std::optional<T> val_;
  std::optional<unexpected<E>> unexpect_;
};

template <class T1, class E1, class T2, class E2>
bool operator==(const expected<T1, E1>& x, const expected<T2, E2>& y) {
  return x.has_value() != y.has_value()
             ? false
             : (!x.has_value() ? x.error() == y.error() : *x == *y);
}

template <class T1, class E1, class T2, class E2>
bool operator!=(const expected<T1, E1>& x, const expected<T2, E2>& y) {
  return x.has_value() != y.has_value()
             ? true
             : (!x.has_value() ? x.error() != y.error() : *x != *y);
}

template <class T1, class E1, class T2>
bool operator==(const expected<T1, E1>& x, const T2& v) {
  return x.has_value() ? *x == v : false;
}

template <class T1, class E1, class T2>
bool operator==(const T2& v, const expected<T1, E1>& x) {
  return x.has_value() ? *x == v : false;
}

template <class T1, class E1, class T2>
bool operator!=(const expected<T1, E1>& x, const T2& v) {
  return x.has_value() ? *x != v : true;
}

template <class T1, class E1, class T2>
bool operator!=(const T2& v, const expected<T1, E1>& x) {
  return x.has_value() ? *x != v : true;
}

template <class T1, class E1, class E2>
bool operator==(const expected<T1, E1>& x, const unexpected<E2>& e) {
  return x.has_value() ? false : x.error() == e.value();
}

template <class T1, class E1, class E2>
bool operator==(const unexpected<E2>& e, const expected<T1, E1>& x) {
  return x.has_value() ? false : x.error() == e.value();
}

template <class T1, class E1, class E2>
bool operator!=(const expected<T1, E1>& x, const unexpected<E2>& e) {
  return x.has_value() ? true : x.error() != e.value();
}

template <class T1, class E1, class E2>
bool operator!=(const unexpected<E2>& e, const expected<T1, E1>& x) {
  return x.has_value() ? true : x.error() != e.value();
}

} // namespace exp

#endif
