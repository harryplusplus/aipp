#pragma once

#include <concepts>
#include <optional>
#include <utility>

template <typename D, typename T>
concept ResourceDeleter = std::invocable<D, T>;

template <typename T, ResourceDeleter<T> D>
class Resource {
 public:
  Resource(T value, D deleter)
      : value_(std::move(value)), deleter_(std::move(deleter)) {}
  ~Resource() {
    if (HasValue()) deleter_(std::move(*value_));
  }

  Resource(Resource&& other) noexcept
      : value_(std::exchange(other.value_, std::nullopt)),
        deleter_(std::move(other.deleter_)) {}

  Resource& operator=(Resource&& other) noexcept {
    if (HasValue()) deleter_(std::move(*value_));
    value_ = std::exchange(other.value_, std::nullopt);
    deleter_ = std::move(other.deleter_);
    return *this;
  }

  Resource(const Resource&) = delete;
  Resource& operator=(const Resource&) = delete;

  T& Get() { return *value_; }
  const T& Get() const { return *value_; }

  explicit operator bool() const { return HasValue(); }
  bool HasValue() const { return value_.has_value(); }

 private:
  std::optional<T> value_;
  D deleter_;
};
