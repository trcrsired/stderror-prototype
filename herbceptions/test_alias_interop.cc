#include "error.h"
#include "my_company_error.h"
#include <cstdio>

// fails{E} -> throws interop with an aliased error type: the auto-propagated
// my_company::win32_errc error must convert to std::error using the alias's
// domain(), so it equals std::win32_errc::file_not_found.

int fails_fn(int x) fails{::my_company::win32_errc} {
  if (x == 0) return failure(::my_company::win32_errc::file_not_found);
  return 2 * x;
}

int caller(int x) throws {
  return try(fails_fn(x));
}

int main()
{
    try
    {
        int v = caller(0);
        fprintf(stderr, "unexpected success %d\n", v);
        return 1;
    }
    catch throws(::std::error e)
    {
        fprintf(stderr, "%d %zu %d\n",
                e == ::std::win32_errc::file_not_found,
                e.code(),
                e == ::my_company::win32_errc::file_not_found);
    }
}
