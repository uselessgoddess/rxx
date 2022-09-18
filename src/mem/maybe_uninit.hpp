#pragma once

#include <bit>

#include "manually_destroy.h"
#include "union.h"

namespace rxx::mem {

struct uninit_t {};

constexpr uninit_t uninit = {};

struct [[maybe_unused]] nulltype {};

/// Safe and uninitialized value
template <typename T>
struct maybe_uninit {
 private:
  // clang-format off
  union_t<
      nulltype,
      manually_destroy<T>
  > place;
  // clang-format on

 public:
  maybe_uninit() noexcept : place(nulltype{}) {
  }

  template <typename... Args>
  explicit maybe_uninit(Args&&... args) noexcept(noexport::nothrow_ctor<T, Args&&...>) {
    place.template emplace<T>(std::forward<Args>(args)...);
  }

  explicit maybe_uninit(uninit_t /*unused*/) noexcept {
    place.template emplace<nulltype>();
  }

  [[nodiscard]] auto assume_init() && noexcept -> T {
    return ptr::read(&**place.template as<T>());
  }

  [[nodiscard]] auto assume_init_ref() noexcept -> T& {
    return **place.template as<T>();
  }

  [[nodiscard]] auto assume_init_const_ref() const noexcept -> const T& {
    return **place.template as<T>();
  }

  auto write(T val) noexcept(noexport::nothrow_ctor<T, T&&>) -> T& {
    return *place.template emplace<T>(std::move(val));
  }

  auto as_ptr() noexcept -> T* {
    return place.template as<T>()->as_ptr();
  }

  auto as_ptr() const noexcept -> const T* {
    return place.template as<const T>()->as_ptr();
  }
};

template <typename T>
  requires(not std::same_as<T, uninit_t>)
maybe_uninit(T) -> maybe_uninit<T>;

}  // namespace rxx::mem