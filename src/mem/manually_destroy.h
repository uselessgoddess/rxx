#pragma once

#include "../ptr/pointee.h"

namespace rxx::mem {

template <typename T>
struct manually_destroy {
 private:
  // `std::array<0, T> same as struct X{}`
  [[no_unique_address]] alignas(alignof(T)) std::array<std::byte, sizeof(T)> buf;

  void destroy() noexcept
  /* don't use exceptions in destructors */ {
    std::destroy_at(ptr::as<T>(&buf));
  }

 public:
  template <typename... Args>
  explicit manually_destroy(Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, Args&&...>) {
    std::construct_at(ptr::as<T>(&buf), std::forward<Args...>(args)...);
  }

  auto operator*() noexcept -> T& {
    return *ptr::as<T>(&buf);
  }

  auto operator*() const noexcept -> const T& {
    return *ptr::as<const T>(&buf);
  }

  auto operator->() noexcept -> T* {
    return as_ptr();
  }

  auto operator->() const noexcept -> const T* {
    return as_ptr();
  }

  auto as_ptr() noexcept -> T* {
    return ptr::as<T>(&buf);
  }

  auto as_ptr() const noexcept -> const T* {
    return ptr::as<const T>(&buf);
  }
};

template <typename T>
manually_destroy(T) -> manually_destroy<T>;

}  // namespace rxx::mem