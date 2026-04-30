#pragma once

#include <concepts>
#include <utility>

template <typename T, typename D>
  requires std::invocable<D, T> && (!std::invocable<D, T&>)
class Resource {
 public:
  Resource(T value, D deleter) : value_(std::move(value)), deleter_(std::move(deleter)) {}
  ~Resource() {
    if (value_) deleter_(std::move(value_));
  }

  Resource(Resource&& other) noexcept
      : value_(std::exchange(other.value_, {})),
        deleter_(std::move(other.deleter_)) {}

  Resource& operator=(Resource&& other) noexcept {
    if (value_) deleter_(std::move(value_));
    value_ = std::exchange(other.value_, {});
    deleter_ = std::move(other.deleter_);
    return *this;
  }

  Resource(const Resource&) = delete;
  Resource& operator=(const Resource&) = delete;

  T& Get() { return value_; }
  const T& Get() const { return value_; }

  explicit operator bool() const { return static_cast<bool>(value_); }

 private:
  T value_;
  D deleter_;
};
