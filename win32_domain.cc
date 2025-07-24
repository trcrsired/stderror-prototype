#include"error.h"
#include<cerrno>
namespace std::error_domains
{

namespace
{
constinit error_domain_singleton __win32_error_domain
{
    .do_equivalent=[](std::size_t cd, error_domain_singleton const* otherdomain,::std::size_t othercd) noexcept
    {
        return __win32_error_domain.do_to_errc(cd) == otherdomain->do_to_errc(othercd);
    },
    .do_name=[](::std::size_t, ::std::error_reporter::char_type_flag, ::std::error_reporter*) noexcept
    {
    },
    .do_message=[](::std::size_t, ::std::error_reporter::char_type_flag, ::std::error_reporter*) noexcept
    {
    },
    .do_to_errc=[](::std::size_t cd) noexcept
    {
        switch(static_cast<::std::win32_errc>(static_cast<::std::uint_least32_t>(cd)))
        {
        case ::std::win32_errc::success: 
            return static_cast<::std::errc>(0);
        case ::std::win32_errc::invalid_function:
            return ::std::errc::invalid_argument;
        case ::std::win32_errc::file_not_found:
            return ::std::errc::no_such_file_or_directory;
        default:
            return ::std::errc::invalid_argument;
        }
    }
#if 0
    .do_throw_dynamic_exception=[](::std::size_t cd, ::std::dynamic_exception_abi)
    {
#if defined(__cpp_exceptions)
        throw ::std::system_error(__win32_error_domain.do_to_errc(cd), ::std::generic_category());
#else
        __builtin_trap();
#endif
    }
#endif
};
}

extern "C"
[[__gnu__::__weak__]]
::std::error_domain_singleton const* __cxa_error_domain_win32() noexcept
{
    return __builtin_addressof(::std::error_domains::__win32_error_domain);
}
}