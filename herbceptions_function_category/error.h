#pragma once
/*
unfinished semantics
*/
#include<type_traits>
#include<cstddef>
#if defined(__cpp_exceptions)
#include<system_error>
#endif
#include<cstdint>


namespace std
{

struct error;

#if 0
enum class legacy_exception_abi
{
itanium=0,
microsoft=1,
#if defined(_MSC_VER)
platform=microsoft
#else
platform=itanium
#endif
};
#endif
struct io_scatter_t
{
    void const* base;
    ::std::size_t len;
};
enum class error_reporter_encoding
{
utf8,
utf16,
utf32,
gb18030,
utfebcdic
};

using error_reporter_io_cookie_function = void (*)(::std::error_reporter_encoding, void*, ::std::io_scatter_t const* , ::std::size_t) noexcept;

struct error_domain_singleton
{
    void (*do_cleanup)(::std::size_t) noexcept = nullptr;
    bool (*do_equivalent)(::std::size_t, ::std::error_domain_singleton const*, ::std::size_t) noexcept = nullptr;
    void (*do_name)(::std::size_t, ::std::error_reporter_encoding, void*, ::std::error_reporter_io_cookie_function) noexcept = nullptr;
    void (*do_message)(::std::size_t, ::std::error_reporter_encoding, void*, ::std::error_reporter_io_cookie_function) noexcept = nullptr;
    ::std::errc (*do_to_errc)(::std::size_t) noexcept = nullptr;
#if 0
// allow old style EH is a bad idea because of ABI plus bloated to every singleton table
    void (*do_throw_dynamic_exception)(::std::size_t, ::std::legacy_exception_abi) = nullptr;
#endif
};

namespace __details
{
template<typename __T>
concept __error_domain_has_domain_alias_type = requires()
{
    typename __T::domain_alias_type;
};
}
// The error_domain customization point. It is declared but not defined: only
// specializations exist. T need not be an enum — any type with an
// error_domain<T> specialization can be thrown via `throw throws`.
template<typename T>
class error_domain;

// std::error is defined by the standard library, but it is not a type users
// can create, copy, or store. The compiler is the only manufacturer: for
// `throw throws e;` it evaluates error_domain<T>::domain() and
// error_domain<T>::code(e) and fabricates the {domain, code} value.
// Consequently:
//   - the default constructor is deleted;
//   - copy/move constructors and assignments are all deleted;
//   - there is only a destructor and helper methods;
//   - all data members are private.
class error
{
public:
    error() = delete;
    error(error const&) = delete;
    error(error&&) = delete;
    error& operator=(error const&) = delete;
    error& operator=(error&&) = delete;

    constexpr ~error() noexcept
    {
        auto __docleanup{__domain_opaque->do_cleanup};
        if (__docleanup)
        {
            __docleanup(__code_opaque);
        }
    }

    [[nodiscard]] constexpr ::std::error_domain_singleton const* domain() const noexcept
    {
        return __domain_opaque;
    }
    [[nodiscard]] constexpr ::std::size_t code() const noexcept
    {
        return __code_opaque;
    }

    template<typename __Other>
    requires (::std::is_class_v<__Other> || ::std::is_enum_v<__Other>)
    constexpr bool do_equivalent(__Other __ec) const noexcept
    {
        using __other_error_domain_type = ::std::error_domain<__Other>;
        if constexpr(::std::__details::__error_domain_has_domain_alias_type<__other_error_domain_type>)
        {
            using __domain_alias_type = typename __other_error_domain_type::domain_alias_type;
            return __domain_opaque->do_equivalent(
                __code_opaque,
                __domain_alias_type::domain(),
                __other_error_domain_type::code(__ec));
        }
        else
        {
            return __domain_opaque->do_equivalent(
                __code_opaque,
                __other_error_domain_type::domain(),
                __other_error_domain_type::code(__ec));
        }
    }
    constexpr ::std::errc do_to_errc() const noexcept
    {
        return __domain_opaque->do_to_errc(__code_opaque);
    }

    void do_throw_legacy_exception() const
#if defined(__cpp_exceptions)
    {
        throw ::std::system_error(static_cast<int>(this->do_to_errc()),::std::generic_category());
    }
#else
    = delete; // legacy exception disabled
#endif

private:
    // The {domain, code} payload, laid out as exactly two words so the value
    // flows through the {void*, size_t} ABI slot unchanged.
    ::std::error_domain_singleton const* __domain_opaque{};
    ::std::size_t __code_opaque{};

    // a magic function compiler will know how to construct it
    explicit constexpr error(void const* __domain, ::std::size_t __code) noexcept
        : __domain_opaque(static_cast<::std::error_domain_singleton const*>(__domain)), __code_opaque(__code)
    {
    }
};

namespace error_domains
{
extern "C"
[[__gnu__::__const__]]
::std::error_domain_singleton const* __cxa_error_domain_win32() noexcept;

extern "C"
[[__gnu__::__const__]]
::std::error_domain_singleton const* __cxa_error_domain_posix() noexcept;


extern "C"
[[__gnu__::__const__]]
::std::error_domain_singleton const* __cxa_error_domain_nt() noexcept;
}


enum class win32_errc :
    ::std::uint_least32_t
{
    success=0,
    invalid_function=1,
    file_not_found=2
};

template<>
class error_domain<::std::win32_errc>
{
public:
    using errc_type = ::std::win32_errc;
    static inline constexpr ::std::error_domain_singleton const* domain() noexcept
    {
        return ::std::error_domains::__cxa_error_domain_win32();
    }
    static inline constexpr ::std::size_t code(errc_type __e) noexcept
    {
        return static_cast<::std::size_t>(static_cast<::std::uint_least32_t>(__e));
    }
};

/*
this is much more flexible than the template specialization one. since different libraries may have the their implementation of errc win32_errc for example. This allow them
to have only one category as long as the code function is a template to help constexpr and potentially reduce binary bloat.
*/

/*
pesudo throws should do
template<typename T>
requires (::std::is_class_v<T> || ::std::is_enum_v<T>)
inline constexpr void pesudo_throws(T x)
{
  using error_domain_type = get_error_domain(::std::error_domain_tag, x);
  throw ::std::error{error_domain_type::domain(), error_domain_type::code(x)};
}

*/

template<typename __Other>
requires (::std::is_class_v<__Other> || ::std::is_enum_v<__Other>)
constexpr bool operator==(::std::error const& __ec, __Other const& __other) noexcept
{
    using __other_error_domain_type = ::std::error_domain<__Other>;
    if constexpr(::std::__details::__error_domain_has_domain_alias_type<__other_error_domain_type>)
    {
        using __other_domain_alias_type = typename __other_error_domain_type::domain_alias_type;
        return __other_error_domain_type::code(__other) == __ec.code() &&
            __other_domain_alias_type::domain() == __ec.domain();
    }
    else
    {
        return __other_error_domain_type::code(__other) == __ec.code() &&
            __other_error_domain_type::domain() == __ec.domain();
    }

}

template<typename __Other>
requires (::std::is_class_v<__Other> || ::std::is_enum_v<__Other>)
constexpr bool operator==(__Other const __other, ::std::error const& __e) noexcept
{
    return __e==__other;
}

}
