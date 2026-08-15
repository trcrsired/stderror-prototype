#include "error.h"
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
