#include"error.h"
#include<cstdio>
#include<cstring>

void some_c_style_function() fails{::std::win32_errc} //use {} not () because it is type
{
	return failure(::std::win32_errc::file_not_found);
}

void f() throws
{
    some_c_style_function(); //ok in C++ no problem since the some c_style_function failure type ::std::win32_errc can construct std::error
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
