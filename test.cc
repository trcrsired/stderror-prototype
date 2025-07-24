#include"error.h"
#include"pesudo_throws.h"
#include<cstdio>

int main()
{
    try
    {
        pesudo_throws(::std::win32_errc::file_not_found);
    }
    catch(::std::error e)
    {
        fprintf(stderr,"%s\n",strerror(static_cast<int>(e.do_to_errc())));
    }
}