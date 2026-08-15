#include "error.h"
// Use a user-named N2289-style aggregate to avoid the auto-copy problem.
struct Result { union { int value; ::std::error error; }; bool failed; };

constexpr int f(int x) throws {
  if (x == 0) throw throws ::std::win32_errc::file_not_found;
  return 2 * x;
}

constexpr int use_catch(int x) {
  Result r = catch fails(f(x));
  if (r.failed) return static_cast<int>(r.error.code());
  return r.value;
}
