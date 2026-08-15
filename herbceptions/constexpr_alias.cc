#include "error.h"
#include "my_company_error.h"

// Constexpr alias mechanism: throw throws my_company::win32_errc::x and catch
// throws(std::error e); e == my_company::win32_errc::x and e.code() must be
// constant-evaluable, with the domain fabricated via the alias's domain().

constexpr int f(int x) throws {
  if (x == 0) throw throws ::my_company::win32_errc::file_not_found;
  return 2 * x;
}

constexpr int use_try(int x) {
  try {
    return f(x);
  } catch throws(::std::error e) {
    if (e == ::my_company::win32_errc::file_not_found)
      return static_cast<int>(e.code());
    return -1;
  }
}

static_assert(use_try(3) == 6, "success");
static_assert(use_try(0) == 2, "failure");

int main() { return 0; }
