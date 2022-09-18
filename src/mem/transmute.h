#pragma once

#include <bit>

#include "../ptr/pointee.h"
#include "maybe_uninit.hpp"

namespace rxx::mem {

template <typename T, typename U>
constexpr auto transmute_select(T src) noexcept -> U {
  if consteval {
    // super strongly bounds
    return std::bit_cast<U>(src);
  } else {
    auto place = maybe_uninit<U>(uninit);
    auto ptr = ptr::as<T>(place.as_ptr());
    ptr::write(ptr, src);
    return std::move(place).assume_init();
  }
}

template <typename U>
constexpr auto transmute(auto src) noexcept -> U {
  using T = decltype(src);

  static_assert(sizeof(T) == sizeof(U));

  if constexpr (std::is_pointer_v<T> && std::is_pointer_v<U>) {
    return unreachable(/* use `transmute_cast` or `as` */);
  } else if constexpr (std::is_reference_v<T> && std::is_reference_v<U>) {
    return unreachable(/* in C++ `T` and `T&` has similar behaviour */);
  } else {
    return transmute_select<T, U>(src);
  }
}

template <typename U>
constexpr auto transmute_copy(const auto& src) noexcept -> U {
  using std::array;
  using std::byte;
  using T = std::remove_cvref_t<decltype(src)>;

  static_assert(sizeof(T) >= sizeof(U));

  auto place = transmute<array<byte, sizeof(T)>>(src);

  if (alignof(U) > alignof(T)) {
    return rxx::ptr::read_unaligned(as<U>(&src));
  } else {
    return rxx::ptr::read(as<U>(&src));
  }
}

}  // namespace rxx::mem
