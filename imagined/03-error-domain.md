# 03 · `error_domain_singleton` and `error_domain<T>`

The magic type `__CPP_STD_ERROR__` carries `{domain, code}`. `domain` points at
a `const` singleton describing *how to interpret* `code`. This doc is about that
singleton and the enum customization that binds it.

## `error_domain_singleton` — a C-ABI function table

Keep it a plain struct of function pointers (a "C vtable"). C++ vtables are
forbidden: they add a vptr, RTTI coupling, and language-specific layout. A
struct of `extern "C"`-friendly pointers is implementable from any language.

```cpp
struct error_domain_singleton {
    // Cross-domain equivalence hook: is `code` equal to `other->code(other_code)`?
    // Consulted by __CPP_STD_ERROR__::do_equivalent(T).
    bool (*do_equivalent)(std::size_t code,
                          error_domain_singleton const* other,
                          std::size_t other_code) noexcept;

    // Human-readable domain name, written through an IO callback.
    void (*do_name)(std::size_t code,
                    std::error_reporter_encoding encoding,
                    void* cookie,
                    std::error_reporter_io_cookie_function cook) noexcept;

    // Human-readable message for `code`, written through an IO callback.
    void (*do_message)(std::size_t code,
                       std::error_reporter_encoding encoding,
                       void* cookie,
                       std::error_reporter_io_cookie_function cook) noexcept;

    // Map to a portable POSIX errc (for strerror / error_code interop).
    std::errc (*do_to_errc)(std::size_t code) noexcept;

    // Optional ownership hook: called by the magic type's destructor when the
    // catch parameter goes out of scope. nullptr for stateless domains.
    void (*do_cleanup)(error_domain_singleton const* domain,
                       std::size_t code) noexcept;

    // Strict-identity check used by operator==(std::error, T). The enum value
    // `t` belongs to *this* domain; the other side is an error's (domain, code).
    // "win32_errc's domain->pass(win32_errc::xxx, e.domain(), e.code())".
    template <typename T>
    requires std::is_enum_v<T>
    constexpr bool pass(T t, error_domain_singleton const* other_domain,
                        std::size_t other_code) const noexcept {
        return this == other_domain &&                 // same domain table
               error_domain<T>::code(t) == other_code;  // same code value
    }
};
```

`pass` is an ordinary member template (not a table entry — it is the C++
implementation of strict identity, defined in terms of `error_domain<T>`), so
`==` routes through the enum's domain without going through the function
pointer table. `do_equivalent` is the *table hook* used for the weaker
cross-domain query.

### `do_cleanup` — the only hook the magic type's destructor consults

```cpp
// Compiler-generated ~__CPP_STD_ERROR__:
// domain() is never null by contract; only do_cleanup is optional.
if (auto* d = this->domain(); d->do_cleanup)
    d->do_cleanup(d, this->code());
```

- `nullptr` → no-op (POSIX/Win32/NT/COM codes): error is a pure value, zero cost.
- non-null → called exactly once when the `catch throws` variable dies.

Because the magic type has no copy/move constructors, there is **no double-call
problem**: the value is fabricated by the compiler, flows through the payload as
two registers, and is materialized exactly once in the catch variable. Ownership
can never be duplicated.

## `error_domain<T>` — the compiler-facing bridge

The prototype's `error_domain<T>` (static `domain()` + `code(e)`) is exactly
right, but it is not a *user* construction path — it is the ABI bridge the
compiler calls to build the magic value from an enum in `throw throws e`:

```cpp
// compiler-emitted for: throw throws e;   (e : enum T)
auto d = error_domain<T>::domain();
auto c = error_domain<T>::code(e);
// fabricate __CPP_STD_ERROR__ from {d, c}
```

Keep the primary template constrained to enums and specialized per domain:

```cpp
template <typename T>
requires std::is_enum_v<T>
struct error_domain;   // not defined — specializations only

template <>
struct error_domain<win32_errc> {
    using errc_type = win32_errc;
    static constexpr error_domain_singleton const* domain() noexcept {
        return error_domains::__cxa_error_domain_win32();
    }
    static constexpr std::size_t code(errc_type e) noexcept {
        return static_cast<std::size_t>(e);
    }
};
```

This keeps the current weak-accessor pattern: `__cxa_error_domain_win32()` is a
weak `extern "C"` function so the platform (or Wine) can override it.

## Factory / accessor conventions

- `error_domains::` namespace holds `extern "C"` weak accessors, one per domain:
  `__cxa_error_domain_posix`, `__cxa_error_domain_win32`,
  `__cxa_error_domain_nt`, `__cxa_error_domain_com`, `__cxa_error_domain_wine`.
- Each returns `error_domain_singleton const*` to a `constinit` singleton in an
  anonymous namespace — zero dynamic init, stable addresses.
- `error_domain<T>::domain()` delegates to the accessor; `code(T)` extracts the
  underlying value (no allocation, no validation).

## Comparison semantics

**`operator==` is strict identity, not equivalence.** It requires both the same
domain table pointer AND the same code value, and it is delegated to **the
enum's domain** via `pass`:

```cpp
template <typename T> requires std::is_enum_v<T>
constexpr bool operator==(std::error e, T t) noexcept {
    // routes through the enum's domain:
    //   win32_domain->pass(win32_errc::file_not_found, e.domain(), e.code())
    return error_domain<T>::domain()->pass(t, e.domain(), e.code());
}
```

`pass` checks `this == other_domain && error_domain<T>::code(t) == other_code`
(see the singleton definition above). The symmetric `T == std::error` delegates
to the same call.

**`do_equivalent` is weaker and separate.** The `error::do_equivalent(T)`
method asks "does this error mean the same thing as enum `t`, possibly from
another domain?" It dispatches through **this error's** `domain()->do_equivalent(
code, other_domain, other_code)`. It is never folded into `==` — `win32_errc::file_not_found`
and `errc::no_such_file_or_directory` may be `do_equivalent` but are never `==`.

`error_domain<T>` exists solely to give the compiler the `{domain, code}`
mapping for `throw throws e` and the enum comparison / equivalence operands.
