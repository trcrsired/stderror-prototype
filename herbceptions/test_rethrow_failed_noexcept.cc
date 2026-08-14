#include"error.h"
#include<cstdio>
#include<cstring>

void f() throws
{
    throw throws ::std::win32_errc::file_not_found;
}

//should fail to compile since the function is marked as noexcept
void g() noexcept
{
    f();
}

int main()
{
    try
    {
        g();
    }
    catch throws(::std::error e)
    {
        fprintf(stderr,"%d\n%s\n",
                e==::std::win32_errc::file_not_found,
                strerror(static_cast<int>(e.do_to_errc())));
    }
}
