#include "error.h"

constexpr int f(int x) throws {
  if (x == 0) throw throws ::std::win32_errc::file_not_found;
  return 2 * x;
}

constexpr int use_catch(int x) {
  auto r = catch fails(f(x));
  if (r.failed) return static_cast<int>(r.error.code());
  return r.value;
}

static_assert(use_catch(3) == 6, "success");
int main(){ return 0; }
