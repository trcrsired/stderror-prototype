#include"error.h"
#include"pesudo_throws.h"

int main()
{
    try
    {
        pesudo_throws(::std::win32_errc::file_not_found);
    }
    catch(::std::error e)
    {
        
    }
}