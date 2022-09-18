#pragma once

#include <optional>

#include "../intrin/unreachable.h"
#include "../mem/transmute.h"

namespace rxx::ptr {

struct bad_not_null : public std::exception {
 public:
  bad_not_null() noexcept = default;

  bad_not_null(const bad_not_null&) = default;
  auto operator=(const bad_not_null&) -> bad_not_null& = default;

  ~bad_not_null() noexcept override = default;

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return "nullptr in not_null constructor";
  }
};

/// noexcept(false) with message
#define except(...) noexcept(false)

struct _priv_unchecked {};

template <typename T>
struct not_null {
 private:
  T* ptr;

 public:
  constexpr explicit not_null(const T& ref) noexcept : ptr(const_cast<T*>(&ref)) {}

  constexpr explicit not_null(T* ptr) except("`ptr` may be nullptr") : ptr(ptr) {
    if (ptr == nullptr) {
      throw bad_not_null{};
    }
  }

  constexpr explicit not_null(_priv_unchecked /*unused*/, const T* ptr) noexcept : ptr(ptr) {}

  template <typename U>
  constexpr explicit operator not_null<U>() noexcept {
    return not_null(_priv_unchecked{}, as<U>(ptr));
  }

  constexpr auto as_ref() const noexcept -> decltype(auto) /* ref */  {
    return *as_ptr();
  }

  constexpr auto as_ptr() const noexcept -> auto /* copy */ {
    return assume_get();
  }

 private:
  constexpr auto assume_get() noexcept -> T* {
    return ptr != nullptr ? ptr : unreachable();
  }

  constexpr auto assume_get() const noexcept -> const T* {
    return ptr != nullptr ? ptr : unreachable();
  }
};

template <ptr::pointee P>
not_null(P) -> not_null<std::remove_pointer_t<P>>;

template <typename T>
  requires std::is_reference_v<T>
not_null(T) -> not_null<T>;

template <typename T>
constexpr auto make_not_null_unchecked(T* ptr) -> not_null<T> {
  return not_null(_priv_unchecked{}, ptr);
}

template <typename T>
constexpr auto make_not_null(T* ptr) -> std::optional<not_null<T>> {
  return ptr != nullptr ? std::optional(make_not_null_unchecked(ptr))
                        : std::optional<not_null<T>>(std::nullopt);
}

template <typename T>
constexpr auto invalid(size_t addr) -> T* {
  return mem::transmute<T*>(addr);
}

template <typename T>
constexpr auto dangling() -> std::optional<not_null<T>> {
  return make_not_null_unchecked(invalid<T>(alignof(T)));
}

#undef except

}  // namespace rxx::ptr