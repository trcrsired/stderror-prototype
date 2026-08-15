#include "error.h"
// constexpr throws with catch throws block handler.
constexpr int f(int x) throws {
  if (x == 0) throw throws ::std::win32_errc::file_not_found;
  return 2 * x;
}

constexpr int use_try(int x) {
  try {
    return f(x);
  } catch throws(::std::error e) {
    return static_cast<int>(e.code());
  }
}

static_assert(use_try(3) == 6, "success");
static_assert(use_try(0) == 2, "failure");

int main(){ return 0; }
