#pragma once
#include<type_traits>
#include<cstddef>
#include<system_error>

namespace std
{

struct error
{
    void *domain_opaque{};
    ::std::size_t code_opaque{};
};

enum class dynamic_exception_abi
{
itanium=0,
microsoft=1,
#if defined(_MSC_VER)
platform=microsoft
#else
platform=itanium
#endif
};

struct io_scatter_t
{
    void const* base;
    ::std::size_t len;
};

struct error_reporter
{
    enum class char_type_flag
    {
        flag_char,
        flag_wchar_t,
        flag_char8_t,
        flag_char16_t,
        flag_char32_t
    };
    virtual void scatter_write(char_type_flag flag, void*, ::std::io_scatter_t const* vecs, ::std::size_t n) noexcept;
};

struct error_domain_singleton
{
    bool (*do_equivalent)(::std::error) noexcept = 0;
    void (*do_name)(std::error, ::std::error_reporter::char_type_flag, ::std::error_reporter&) noexcept = 0;
    void (*do_message)(std::error, ::std::error_reporter::char_type_flag, ::std::error_reporter&) noexcept = 0;
    ::std::errc (*do_to_errc)(std::error) noexcept = 0;
    void (*do_throw_dynamic_exception)(::std::error, ::std::dynamic_exception_abi) noexcept = 0;
};

namespace error_domains
{
extern "C" ::std::error_domain_singleton* __error_domain_win32() noexcept;
extern "C" ::std::error_domain_singleton* __error_domain_posix() noexcept;
extern "C" ::std::error_domain_singleton* __error_domain_nt() noexcept;
}

template<typename T>
requires (::std::is_enum_v<T>)
class error_domain;


enum class win32_errc :
    ::std::uint_least32_t
{
};

template<>
struct error_domain<::std::win32_errc>
{
    using errc_type = ::std::win32_errc;
    constexpr ::std::error_domain_singleton* domain() noexcept
    {
        return ::std::error_domains::__error_domain_win32();
    }
    constexpr ::std::size_t code(errc_type e) noexcept
    {
        return static_cast<::std::size_t>(static_cast<::std::uint_least32_t>(e));
    }
};

template<typename T>
requires (::std::is_enum_v<T>)
constexpr bool operator==(::std::error e, T t) noexcept
{
    using error_type = typename ::std::error_domain<T>::error_type;
    return error_type::code(t) == e.code_opaque &&
        error_type::domain() == e.domain_opaque;
}

template<typename T>
requires (::std::is_enum_v<T>)
constexpr bool operator==(T t, ::std::error e) noexcept
{
    return e==t;
}

template<typename T>
requires (::std::is_enum_v<T>)
constexpr bool operator!=(::std::error e, T t) noexcept
{
    return !(e==t);
}

template<typename T>
requires (::std::is_enum_v<T>)
constexpr bool operator!=(T t, ::std::error e) noexcept
{
    return !(e==t);
}

}
