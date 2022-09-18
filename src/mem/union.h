#pragma once

#include "manually_destroy.h"

namespace rxx::mem {

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
  static_assert((trivially_destroy<Ts> && ...),
                "unions cannot contain fields that may need dropping\n"
                "a type is guaranteed not to need destroying when it is `trivially`, or when it is "
                "the special "
                "`manually_destroy<_>` type");

  [[no_unique_address]] alignas(max(alignof(Ts)...)) std::array<std::byte, max(sizeof(Ts)...)> buf;

  // template <typename U>
  // explicit union_t(U u) {
  //   emplace<U>(u);
  // }

  template <any_of<Ts...> U, typename... Args>
  auto emplace(Args&&... args) noexcept(noexport::nothrow_ctor<U, Args&&...>) -> U& {
    static_assert((trivially_destroy<Ts> || ...));
    return *std::construct_at(as<U>(), std::forward<Args>(args)...);
  }

  template <typename U, typename... Args>
  auto emplace(Args&&... args) noexcept(noexport::nothrow_ctor<U, Args&&...>)
      -> manually_destroy<U>&
    requires any_of<manually_destroy<U>,
                    Ts...> {
    return emplace<manually_destroy<U>>(std::forward<Args>(args)...);
  }

  template <any_of<Ts...> U>
  [[nodiscard]] auto as() noexcept -> U* {
    return ptr::as<U>(&buf);
  }

  template <any_of<Ts...> U>
  [[nodiscard]] auto as() const noexcept -> const U* {
    return ptr::as<const U>(&buf);
  }

  template <typename U>
  [[nodiscard]] auto as() noexcept -> manually_destroy<U>*
    requires any_of<manually_destroy<U>,
                    Ts...> {
    return ptr::as<manually_destroy<U>>(&buf);
  }

  template <typename U>
  [[nodiscard]] auto as() const noexcept -> const manually_destroy<U>*
    requires any_of<manually_destroy<U>,
                    Ts...> {
    return ptr::as<const manually_destroy<U>>(&buf);
  }
};

}  // namespace rxx::mem