#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <tuple>
#include <type_traits>

#include "ptr/ptr.hpp"
#include "mem/mem.hpp"

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

}  // namespace rxx