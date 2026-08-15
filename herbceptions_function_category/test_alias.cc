#include "error.h"
#include "my_company_error.h"
#include <cstdio>

// Alias mechanism: my_company::win32_errc shares the std::win32_errc category
// via error_domain<my_company::win32_errc>::domain_alias_type. The compiler
// must use the alias's domain() when fabricating the std::error for
// `throw throws my_company::win32_errc::x`, and e == my_company::win32_errc::x
// must compare against the aliased domain.

int main()
{
    try
    {
        throw throws ::my_company::win32_errc::file_not_found;
    }
    catch throws(::std::error e)
    {
        fprintf(stderr, "%d %zu\n",
                e == ::my_company::win32_errc::file_not_found,
                e.code());
    }
}
