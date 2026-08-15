#pragma once

namespace std
{
/*
A carrier type for error
*/
class coroutine_error
{
    ::std::error_domain_singleton const*  __domain_opaque;
    ::std::size_t __code_opaque;
public:
    coroutine_error() noexcept = delete;
    coroutine_error(coroutine_error const&) = delete;
    coroutine_error& operator=(coroutine_error const&) = delete;
    constexpr coroutine_error(coroutine_error&& __other) noexcept:
        __domain_opaque{__other.__domain_opaque},__code_opaque{__other.__code_opaque}
    {
        __other.__domain_opaque = nullptr;
        __other.__code_opaque = 0;
    }
    constexpr coroutine_error& operator=(coroutine_error&& __other) noexcept
    {
        if(this == __builtin_addressof(__other))
        {
            return *this;
        }
        if(__domain_opaque)
        {
            auto __docleanup{__domain_opaque->do_cleanup};
            if (__docleanup)
            {
                __docleanup(__code_opaque);
            }
        }
        this->__domain_opaque = __other.__domain_opaque;
        this->__code_opaque = __other.__code_opaque;
        __other.__domain_opaque = nullptr;
        __other.__code_opaque = 0;
        return *this;
    }
    constexpr ~coroutine_error()
    {
        if(__domain_opaque)
        {
            auto __docleanup{__domain_opaque->do_cleanup};
            if (__docleanup)
            {
                __docleanup(__code_opaque);
            }
        }
    }
    constexpr operator bool() noexcept
    {
        return __domain_opaque;
    }
};

}
