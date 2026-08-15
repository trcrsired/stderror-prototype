#include "cxa_exception.h"

#include <exception>
#include <new>
#include <stdexcept>
#include <system_error>
#include <typeinfo>

#if defined(__cpp_exceptions)
extern "C" void __cxa_decrement_exception_refcount(void*) noexcept;
extern "C" void __cxa_increment_exception_refcount(void*) noexcept;
extern "C" void __cxa_rethrow_primary_exception(void*) noexcept;
#endif

namespace
{

// The __cxa_exception header precedes the thrown object in the same
// allocation ([header][thrown object]). Given the thrown-object pointer (the
// code / catch value), the header is at thrown_object - 1 (in units of the
// FULL __cxa_exception) and holds the std::type_info* of the dynamic type.
// This layout mirrors libc++abi's __cxa_exception on LP64 Itanium.
struct __cxa_exception_layout
{
    void* reserve;                          //  0
    ::std::size_t referenceCount;           //  8
    ::std::type_info* exceptionType;        // 16
    void (*exceptionDestructor)(void*);     // 24
    void (*unexpectedHandler)();            // 32
    void (*terminateHandler)();             // 40
    void* nextException;                    // 48
    int handlerCount;                       // 56
    int handlerSwitchValue;                 // 60
    unsigned char const* actionRecord;      // 64
    unsigned char const* languageSpecificData; // 72
    void* catchTemp;                        // 80
    void* adjustedPtr;                      // 88
    struct _Unwind_Exception
    {
        unsigned long long exception_class; // 96
        void (*exception_cleanup)(int, void*); // 104
        unsigned long private_1;            // 112
        unsigned long private_2;            // 120
    } unwindHeader;                         // sizeof = 128
};
static_assert(sizeof(__cxa_exception_layout) == 128, "unexpected __cxa_exception layout");

constexpr ::std::type_info* cxa_type_info_of(::std::size_t cd) noexcept
{
    if (cd == 0)
        return nullptr;
    __cxa_exception_layout const* header =
        reinterpret_cast<__cxa_exception_layout const*>(cd) - 1;
    return header->exceptionType;
}

// Itanium ABI catch-matching primitive (the same one the personality routine
// uses): returns whether a handler of type T could catch the thrown object,
// storing the adjusted pointer in __obj.
template<typename T>
bool is_catchable_as(::std::size_t cd, void*& __obj) noexcept
{
    ::std::type_info const* thrown = cxa_type_info_of(cd);
    if (!thrown)
        return false;
    void* adjusted = reinterpret_cast<void*>(cd);
    if (thrown->__do_catch(&typeid(T), &adjusted, 0u))
    {
        __obj = adjusted;
        return true;
    }
    return false;
}

::std::errc errc_of(::std::error_code const& __ec) noexcept
{
    if (__ec.category() == ::std::generic_category())
        return static_cast<::std::errc>(__ec.value());
    // Unknown category; fall back to a POSIX-like mapping via the message.
    return ::std::errc::io_error;
}

void write_string(::std::size_t, ::std::error_reporter_encoding encoding, void* cookie,
                  ::std::error_reporter_io_cookie_function cookfun,
                  void const* base, ::std::size_t len) noexcept
{
    ::std::io_scatter_t v{base, len};
    cookfun(encoding, cookie, __builtin_addressof(v), 1u);
}

}

namespace std::error_domains
{

namespace
{
constinit ::std::error_domain_singleton __cxa_exception_error_domain
{
    // The code is the thrown-object pointer (the __cxa catch value). When the
    // error value dies, release the reference; this destroys the exception
    // object exactly when the last reference goes away. Incrementing on
    // capture is the responsibility of the boundary that captured it,
    // mirroring __cxa_current_primary_exception.
    .do_cleanup=[](::std::size_t cd) noexcept
    {
#if defined(__cpp_exceptions)
        __cxa_decrement_exception_refcount(reinterpret_cast<void*>(cd));
#endif
    },
    // Two cxa exceptions are equivalent when they are the same exception
    // object (same catch value). Cross-domain equivalence (e.g. comparing to
    // a plain errc) falls back to identity as well; a cxa C++ exception
    // carries no errno mapping.
    .do_equivalent=[](::std::size_t cd, ::std::error_domain_singleton const*, ::std::size_t othercd) noexcept
    {
        return cd == othercd;
    },
    // The domain name is "cxa_exception", with the dynamic C++ type name
    // obtained through RTTI, e.g. "cxa_exception(std::runtime_error)".
    .do_name=[](::std::size_t cd, ::std::error_reporter_encoding encoding, void* cookie, ::std::error_reporter_io_cookie_function cookfun) noexcept
    {
        ::std::type_info const* thrown = cxa_type_info_of(cd);
        // "cxa_exception(" prefix
        ::std::io_scatter_t v[3];
        switch(encoding)
        {
        case ::std::error_reporter_encoding::utf16:
        {
            v[0].base=u"cxa_exception(";
            v[0].len=14*sizeof(char16_t);
        }
        case ::std::error_reporter_encoding::utf32:
        {
            v[0].base=U"cxa_exception(";
            v[0].len=14*sizeof(char32_t);
        }
        case ::std::error_reporter_encoding::utfebcdic:
        {
            // "cxa_exception(" in EBCDIC cp037
            v[0].base="\x83\xA7\x81\x40\x85\x97\x85\x83\xA3\x89\x98\x83\xA3\x95\x4D";
            v[0].len=15;
        }
        default:
        {
            v[0].base=u8"cxa_exception(";
            v[0].len=14;
        }
        }
        // C++ EH name via RTTI: the mangled typeinfo name.
        v[1].base=thrown ? static_cast<void const*>(thrown->name()) : static_cast<void const*>("?");
        v[1].len=thrown ? __builtin_strlen(thrown->name()) : 1;
        // suffix ")"
        v[2].base=")";
        v[2].len=1;
        cookfun(encoding, cookie, v, 3u);
    },
    // The message is the what() string when the object is a std::exception.
    .do_message=[](::std::size_t cd, ::std::error_reporter_encoding encoding, void* cookie, ::std::error_reporter_io_cookie_function cookfun) noexcept
    {
        void* obj = nullptr;
        if (is_catchable_as<::std::exception>(cd, obj))
        {
            ::std::exception const* e = static_cast<::std::exception const*>(obj);
            char const* what = e->what();
            write_string(cd, encoding, cookie, cookfun, what, __builtin_strlen(what));
        }
    },
    .do_to_errc=[](::std::size_t cd) noexcept -> ::std::errc
    {
        // Map standard library exceptions to errc via RTTI / dynamic_cast.
        void* obj = nullptr;
        if (is_catchable_as<::std::system_error>(cd, obj))
            return static_cast<::std::errc>(
                errc_of(static_cast<::std::system_error const*>(obj)->code()));
        if (is_catchable_as<::std::bad_alloc>(cd, obj))
            return ::std::errc::not_enough_memory;
        if (is_catchable_as<::std::length_error>(cd, obj))
            return ::std::errc::value_too_large;
        if (is_catchable_as<::std::out_of_range>(cd, obj))
            return ::std::errc::result_out_of_range;
        if (is_catchable_as<::std::overflow_error>(cd, obj))
            return ::std::errc::value_too_large;
        if (is_catchable_as<::std::underflow_error>(cd, obj))
            return ::std::errc::result_out_of_range;
        if (is_catchable_as<::std::domain_error>(cd, obj))
            return ::std::errc::argument_out_of_domain;
        if (is_catchable_as<::std::invalid_argument>(cd, obj))
            return ::std::errc::invalid_argument;
        // Any other std::exception: unclassifiable.
        return ::std::errc::io_error;
    },
    // Rethrow the captured legacy C++ exception. The code is the thrown-object
    // pointer, so the ABI-native rethrow __cxa_rethrow_primary_exception
    // re-throws that exact object.
#if defined(__cpp_exceptions)
    .do_throw_cxa_exception=[](::std::size_t cd, ::std::cxa_exception_abi abi)
    {
        if (abi == ::std::cxa_exception_abi::itanium)
        {
            __cxa_rethrow_primary_exception(reinterpret_cast<void*>(cd));
        }
    }
#endif
};
}

extern "C"
[[__gnu__::__weak__]]
::std::error_domain_singleton const* __cxa_error_domain_cxa_exception_code() noexcept
{
    return __builtin_addressof(::std::error_domains::__cxa_exception_error_domain);
}

}
