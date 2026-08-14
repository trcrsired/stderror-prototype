#pragma once
#include<type_traits>
#include<cstddef>
#include<system_error>
#include<cstdint>

namespace std
{

struct error;

#if 0
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
    void (*do_cleanup)(::std::size_t) noexcept = 0;
    bool (*do_equivalent)(::std::size_t, error_domain_singleton const*, ::std::size_t) noexcept = 0;
    void (*do_name)(::std::size_t, ::std::error_reporter_encoding, void*, ::std::error_reporter_io_cookie_function) noexcept = 0;
    void (*do_message)(::std::size_t, ::std::error_reporter_encoding, void*, ::std::error_reporter_io_cookie_function) noexcept = 0;
    ::std::errc (*do_to_errc)(::std::size_t) noexcept = 0;
#if 0
// allow old style EH is a bad idea
    void (*do_throw_dynamic_exception)(::std::size_t, ::std::dynamic_exception_abi) = 0;
#endif
};

// The error_domain customization point. It is declared but not defined: only
// specializations exist. T need not be an enum — any type with an
// error_domain<T> specialization can be thrown via `throw throws`.
template<typename T>
struct error_domain;

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
        auto docleanup{domain_opaque->do_cleanup};
        if (docleanup)
        {
            docleanup(code_opaque);
        }
    }

    [[nodiscard]] constexpr ::std::error_domain_singleton const* domain() const noexcept
    {
        return domain_opaque;
    }
    [[nodiscard]] constexpr ::std::size_t code() const noexcept
    {
        return code_opaque;
    }

    template<typename T>
    requires (::std::is_class_v<T> || ::std::is_enum_v<T>)
    constexpr bool do_equivalent(T ec) const noexcept
    {
        using other_error_domain_type = ::std::error_domain<T>;
        return domain_opaque->do_equivalent(
            code_opaque,
            other_error_domain_type::domain(),
            other_error_domain_type::code(ec));
    }
    constexpr ::std::errc do_to_errc() const noexcept
    {
        return domain_opaque->do_to_errc(code_opaque);
    }
    constexpr void do_throw_dynamic_exception() const
    {
#if defined(__cpp_exceptions)
        throw ::std::system_error(static_cast<int>(this->do_to_errc()),::std::generic_category());
#else
        ::std::abort();
#endif
    }

private:
    // The {domain, code} payload, laid out as exactly two words so the value
    // flows through the {void*, size_t} ABI slot unchanged.
    ::std::error_domain_singleton const* domain_opaque{};
    ::std::size_t code_opaque{};

    // a magic function compiler will know how to construct it
    explicit constexpr error(void const* domain, ::std::size_t code) noexcept
        : domain_opaque(static_cast<::std::error_domain_singleton const*>(domain)), code_opaque(code)
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
struct error_domain<::std::win32_errc>
{
    using errc_type = ::std::win32_errc;
    static inline constexpr ::std::error_domain_singleton const* domain() noexcept
    {
        return ::std::error_domains::__cxa_error_domain_win32();
    }
    static inline constexpr ::std::size_t code(errc_type e) noexcept
    {
        return static_cast<::std::size_t>(static_cast<::std::uint_least32_t>(e));
    }
};

template<typename T>
requires (::std::is_class_v<T> || ::std::is_enum_v<T>)
constexpr bool operator==(::std::error const& e, T t) noexcept
{
    using error_type = typename ::std::error_domain<T>;
    return error_type::code(t) == e.code() &&
        error_type::domain() == e.domain();
}

template<typename T>
requires (::std::is_class_v<T> || ::std::is_enum_v<T>)
constexpr bool operator==(T t, ::std::error const& e) noexcept
{
    return e==t;
}

}
