#include "error.h"

constexpr int f(int x) throws {
  if (x == 0) throw throws ::std::win32_errc::file_not_found;
  return 2 * x;
}

// `catch throws` binds the fabricated std::error; domain()/code() and the
// value equality `e == win32_errc::x` all work in constant expressions.
constexpr int use_try(int x) {
  try {
    return f(x);
  } catch throws(::std::error e) {
    if (e == ::std::win32_errc::file_not_found)
      return static_cast<int>(e.code());
    return -1;
  }
}

static_assert(use_try(3) == 6, "success");
static_assert(use_try(0) == 2, "failure");

// Cross-domain distinctness: each error_domain<T>::domain() gets a unique
// opaque pointer at compile time.
static_assert(::std::error_domain<::std::win32_errc>::domain() != nullptr,
              "domain is non-null");
int main(){ return 0; }
