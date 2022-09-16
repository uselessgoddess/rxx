#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <tuple>
#include <type_traits>

#include "ptr/ptr.hpp"

namespace rxx {


#define crt_str_impl($str_lit) L##$str_lit
#define crt_str($str_lit) crt_str_impl($str_lit)

template <typename char_t>
auto concreate_or_default(auto... msg) {
  static_assert(sizeof...(msg) <= 1);
  if constexpr (sizeof...(msg) == 0) {
    if constexpr (std::same_as<char_t, wchar_t>) {
      return L"assert failed";
    } else {
      return "assert failed";
    }
  } else {
    return std::get<0>(std::make_tuple(msg...));
  }
}

template <typename char_t>
[[noreturn]] auto debug_assertion(bool cond, const char_t* exp, const char_t* msg) {
#ifndef NDEBUG
  if (!cond) {
    if constexpr (std::same_as<char_t, wchar_t>) {
      auto buf = std::wstring{};
      buf.append(exp);
      buf.append(L"\n`");
      buf.append(msg);
      buf.append(L"`\n");
      _wassert(buf.data(), crt_str(__FILE__), __LINE__);
    } else {
      auto buf = std::string{};
      buf.append(exp);
      buf.append("\n`");
      buf.append(msg);
      buf.append("`\n");
      _assert(buf.data(), __FILE__, __LINE__);
    }
  }
#endif
}

#if defined(_UNICODE) || defined(UNICODE)
#define debug_assert($exp, ...)              \
  rxx::debug_assertion($exp, crt_str(#$exp), \
                       rxx::concreate_or_default<wchar_t>(L"assertion failed", __VA_ARGS__))
#else /* not unicode */
#define debug_assert($exp, ...) \
  rxx::debug_assertion($exp, #$exp, rxx::concreate_or_default<char>(__VA_ARGS__))
#endif /* _UNICODE||UNICODE */

template <typename T>
struct manually_destroy {
 private:
  // `std::array<0, T> same as struct X{}`
  [[no_unique_address]] alignas(alignof(T)) std::array<std::byte, sizeof(T)> buf;

  constexpr void destroy() noexcept
  /* don't use exceptions in destructors */ {
    std::destroy_at(reinterpret_cast<T*>(&buf));
  }

 public:
  template <typename... Args>
  constexpr explicit manually_destroy(Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, Args&&...>) {
    std::construct_at(reinterpret_cast<T*>(&buf), std::forward<Args...>(args)...);
  }

  constexpr auto operator*() noexcept -> T& {
    return *reinterpret_cast<T*>(&buf);
  }

  constexpr auto operator*() const noexcept -> const T& {
    return *reinterpret_cast<const T*>(&buf);
  }

  constexpr auto operator->() noexcept -> T* {
    return reinterpret_cast<T*>(&buf);
  }

  constexpr auto operator->() const noexcept -> const T* {
    return reinterpret_cast<const T*>(&buf);
  }
};

template <typename T>
manually_destroy(T) -> manually_destroy<T>;

// we cannot use the following impl:
// void forget(auto value) {
//   manually_destroy(std::move(value));
// }
// because:
// auto x = std::string{"hello"};
// forget(x);
// has no sense
void forget(auto&& value) {
  static_assert(std::is_rvalue_reference_v<decltype(value)>,
                "\n`T` must be an rvalue reference\n"
                "forgetting a copy doesn't make sense");
  manually_destroy(std::forward<decltype(value)>(value));
}

template <typename T, typename... Ts>
concept any_of = (std::same_as<T, Ts> || ...);

template <typename T>
concept trivially_destroy = std::is_trivially_destructible_v<T>;

// gcc and clang lib `ranges::max` not work with {Ts...}
consteval auto max(auto... xs) -> size_t {
  return std::ranges::max(std::array{xs...});
}

namespace noexport {
// the shortest alias without `is_` and `_v`
template <typename T, typename... Ts>
concept nothrow_ctor = std::is_nothrow_constructible_v<T, Ts...>;
}  // namespace noexport

template <typename... Ts>
struct union_t {
  static_assert(
      (trivially_destroy<Ts> && ...),
      "unions cannot contain fields that may need dropping\n"
      "a type is guaranteed not to need destroying when it is `trivially`, or when it is the special "
      "`manually_destroy<_>` type");

  [[no_unique_address]] alignas(max(alignof(Ts)...)) std::array<std::byte, max(sizeof(Ts)...)> buf;

  template <any_of<Ts...> U, typename... Args>
  auto emplace(Args&&... args) noexcept(noexport::nothrow_ctor<U, Args&&...>) -> U& {
    static_assert((trivially_destroy<Ts> || ...));
    return *std::construct_at(as<U>(), std::forward<Args>(args)...);
  }

  template <typename U, typename... Args>
  auto emplace(Args&&... args) noexcept(noexport::nothrow_ctor<U, Args&&...>) -> manually_destroy<U>&
    requires any_of<manually_destroy<U>,
                    Ts...> {
    return emplace<manually_destroy<U>>(std::forward<Args>(args)...);
  }

  template <any_of<Ts...> U>
  [[nodiscard]] auto as() noexcept -> U* {
    return reinterpret_cast<U*>(&buf);
  }

  template <any_of<Ts...> U>
  [[nodiscard]] auto as() const noexcept -> const U* {
    return reinterpret_cast<const U*>(&buf);
  }

  template <typename U>
  [[nodiscard]] auto as() noexcept -> manually_destroy<U>*
    requires any_of<manually_destroy<U>,
                    Ts...> {
    return reinterpret_cast<manually_destroy<U>*>(&buf);
  }

  template <typename U>
  [[nodiscard]] auto as() const noexcept -> const manually_destroy<U>*
    requires any_of<manually_destroy<U>,
                    Ts...> {
    return reinterpret_cast<const manually_destroy<U>*>(&buf);
  }
};

struct uninit_t {};

constexpr uninit_t uninit = {};

struct [[maybe_unused]] nulltype {};

template <typename T>
struct maybe_uninit {
 private:
  union_t<nulltype, manually_destroy<T>> place;

 public:
  template <typename... Args>
  explicit maybe_uninit(Args&&... args) noexcept(noexport::nothrow_ctor<T, Args&&...>) {
    place.template emplace<T>(std::forward<Args>(args)...);
  }

  explicit maybe_uninit(uninit_t /*unused*/) noexcept {
    place.template emplace<nulltype>();
  }

  [[nodiscard]] auto assume_init() && noexcept -> T {
    return ptr::read(**place.template as<T>());
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
};

template <typename T>
  requires(not std::same_as<T, uninit_t>)
maybe_uninit(T) -> maybe_uninit<T>;

}  // namespace rxx