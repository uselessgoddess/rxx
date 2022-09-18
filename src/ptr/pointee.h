#pragma once

#include <type_traits>

#include "../mem/mem.hpp"

namespace rxx::ptr {

template <typename T>
concept pointee = std::is_pointer_v<T>;

void* voidify(auto&& value) noexcept {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  return const_cast<void*>(reinterpret_cast<const volatile void*>(std::addressof(value)));
}

template <typename>
constexpr bool _false = false;

// `auto*` is pointee
template <ptr::pointee U>
// for true C++ users
constexpr auto transmute_cast(auto* src) noexcept -> U {
  if consteval {
    unreachable(
        /*"Someday it may become constexpr."
        "But not now*/
    );
  }

  U place{};
  ::new (voidify(place)) decltype(src)(src);
  return place;
}

template <typename U>
// for pointers
constexpr auto as(auto* src) noexcept -> U* {
  return transmute_cast<U*>(src);
}

/// move raw pointer into value
// read can impl via `maybe_uninit` but `maybe_uninit` require `read`
// use C++ magic
template <typename T>
constexpr auto read(const T* ptr) -> T {
  return std::move(*ptr);
}

template <typename T>
constexpr auto read_unaligned(const T* ptr) -> T {
  return unreachable();
  // maybe_uninit<T>
}

template <typename T>
auto _is_aligned_and_not_null(const T* ptr) -> bool {
  return ptr != nullptr && (reinterpret_cast<size_t>(ptr) & alignof(T) - 1) == 0;
}

template <typename T>
auto _is_nonoverlapping(const T* src, T* dst, size_t count) -> bool {
  auto size = sizeof(T) * count;
  if (src > dst) {
    return src - dst >= size;
  } else {
    return dst - src <= size;
  };
}

template <typename T>
constexpr void copy(const T* src, T* dst, size_t count) {
  if !consteval {
    debug_assert(_is_aligned_and_not_null(src) && _is_aligned_and_not_null(dst) &&
                 _is_nonoverlapping(src, dst, count));
    std::memmove(dst, src, count * sizeof(T));
  } else {
    if (dst > src) {
      for (size_t i = count; i > 0; i -= sizeof(T)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        dst[i] = src[i];
      }
    } else {
      for (size_t i = 0; i < count; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        dst[i] = src[i];
      }
    }
  }
}

template <typename T>
constexpr void copy_nonoverlapping(const T* src, T* dst, size_t count) {
  if !consteval {
    // debug_assert(_is_aligned_and_not_null(src) && _is_aligned_and_not_null(dst));
    std::memcpy(dst, src, count * sizeof(T));
  } else {
    for (size_t i = 0; i < count; i += sizeof(T)) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      dst[i] = src[i];
    }
  }
}

template <typename T>
constexpr void write(T* ptr, T val) {
  copy_nonoverlapping(&val, ptr, 1);
}

template <typename T>
constexpr void write_unaligned(T* dst, T src) {
  copy_nonoverlapping<uint8_t>(&src, dst, sizeof(T));
}

}  // namespace rxx::ptr