#include"error.h"
#include<cstdio>
#include<cstring>

void f() throws(true)
{
    throw throws ::std::win32_errc::file_not_found;
}

void g() throws
{
    f();
}

int main()
{
     g(); // only main function allows to avoid catching the herbceptions
	  // if the function fails, the compiler will generate __builtin_trap()
	  // to terminate the program
}
