#pragma once

#include <exception>

// constructor is deleted then:
// NOLINTNEXTLINE(*-special-member-functions)
struct [[maybe_unused]] unreachable_t final {
  constexpr unreachable_t() = delete;
  constexpr unreachable_t(const unreachable_t& /*unused*/) noexcept = default;

  [[noreturn]] constexpr static auto unreachable_impl() -> unreachable_t;

  template <typename T>
  // we want implicit conversion because we cannot create `unreachable_t`
  // NOLINTNEXTLINE(*-explicit-*)
  [[noreturn]] constexpr operator T&() const noexcept {
    unreachable_impl();
  }
  template <typename T>
  // we want implicit conversion because we cannot create `unreachable_t`
  // NOLINTNEXTLINE(*-explicit-*)
  [[noreturn]] constexpr operator T&&() const noexcept {
    unreachable_impl();
  }
};

[[noreturn]] inline constexpr auto unreachable() -> unreachable_t {
  if consteval {
    std::terminate(/* unreachable is not allowed in constexpr */);
  } else {
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)  // GCC, Clang, ICC
    __builtin_unreachable();
#elifdef _MSC_VER  // MSVC
    __assume(false);
#else
    std::abort();
#endif
  }
}

[[noreturn]] inline constexpr auto unreachable_t::unreachable_impl() -> unreachable_t {
  unreachable();
}
