#include"error.h"
#include<cstdio>
#include<cstring>

void f() throws
{
    throw throws ::std::win32_errc::file_not_found;
}

void g() throws
{
try
{
	f();
}
catch throws(std::error e)
{
	throw throws e;
}
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
