// -----------------------------------------------------------------------
// Resource — generic RAII wrapper for any resource with a deleter.
//
//   Resource log_win{newwin(...), delwin};
//   scrollok(log_win, TRUE);      // implicit conversion to T
//
// Movable but not copyable.
// -----------------------------------------------------------------------
#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

template <typename T, typename D>
  requires std::invocable<D, T> && std::is_nothrow_invocable_v<D, T>
class Resource {
 public:
  Resource(T value, D deleter) : value_(value), deleter_(std::move(deleter)) {}
  ~Resource() noexcept {
    if (value_) deleter_(value_);
  }

  // Movable
  Resource(Resource&& other)
      : value_(std::exchange(other.value_, {})),
        deleter_(std::move(other.deleter_)) {}
  Resource& operator=(Resource&& other) {
    if (value_) deleter_(value_);
    value_ = std::exchange(other.value_, {});
    deleter_ = std::move(other.deleter_);
    return *this;
  }

  Resource(const Resource&) = delete;
  Resource& operator=(const Resource&) = delete;

  // Access
  T& get() { return value_; }
  const T& get() const { return value_; }
  operator T() const { return value_; }
  explicit operator bool() const { return static_cast<bool>(value_); }

 private:
  T value_{};
  D deleter_;
};
