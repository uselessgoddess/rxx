#pragma once

#include <bit>

#include "manually_destroy.h"
#include "maybe_uninit.hpp"
#include "transmute.h"
#include "union.h"

namespace rxx::mem {

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

}  // namespace rxx::mem