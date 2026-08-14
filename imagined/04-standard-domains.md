# 04 · Standard domains and enum coverage

The magic `error` type is useless without a set of well-known domains. The
prototype only has `win32_errc` with three values; a real library needs the
platforms' full error spaces plus the portable POSIX one.

## Domain inventory

| Domain | Enum | Accessor | Notes |
|--------|------|----------|-------|
| POSIX | `std::errc` | `__cxa_error_domain_posix()` | already exists in `<system_error>` |
| Win32 | `std::win32_errc` | `__cxa_error_domain_win32()` | `GetLastError()` values |
| NT | `std::nt_errc` | `__cxa_error_domain_nt()` | `NTSTATUS` values |
| COM | `std::com_errc` | `__cxa_error_domain_com()` | `HRESULT` values |
| Wine | `std::wine_errc` | `__cxa_error_domain_wine()` | host POSIX errno under Wine |

Each is a `constinit` singleton in an anonymous namespace, exposed through a
weak `extern "C"` accessor so the platform can override it
(`imagined/03-error-domain.md`).

## Enum coverage

The prototype's `win32_errc` has 3 values; a real library enumerates the full
documented space. The underlying type must match the OS (`uint_least32_t` for
Win32, `LONG`-compatible for NTSTATUS/HRESULT) so `code(T)` is a trivial cast:

```cpp
enum class win32_errc : std::uint_least32_t {
    success                  = 0,
    invalid_function         = 1,
    file_not_found           = 2,
    path_not_found           = 3,
    too_many_open_files      = 4,
    access_denied            = 5,
    invalid_handle           = 6,
    // ... full table
};
```

## `do_to_errc` — preserving information

The prototype collapses unknown codes to `invalid_argument`, losing data.
Better: return a portable `errc` when a canonical mapping exists, and otherwise
fall back to a raw value the caller can still see via `code()`:

```cpp
static std::errc to_errc(std::uint_least32_t cd) noexcept {
    switch (static_cast<win32_errc>(cd)) {
    case win32_errc::success:      return static_cast<std::errc>(0);
    case win32_errc::file_not_found:
    case win32_errc::path_not_found:
        return std::errc::no_such_file_or_directory;
    case win32_errc::access_denied: return std::errc::permission_denied;
    // ...
    default:
        // No canonical errc. Still expose the raw code through error::code().
        return static_cast<std::errc>(cd);   // or a reserved sentinel
    }
}
```

The point of `do_to_errc` is interop (`strerror`, `std::error_code`), so the
mapping is allowed to be lossy — but the loss must never hide the original code.

## `do_equivalent` — code identity first

Two comparison paths exist, and they must not be confused:

- **`operator==` uses `pass`** (strict identity: same domain table + same code),
  defined once on `error_domain_singleton` and routed through the enum's domain.
- **`error::do_equivalent(T)`** is the weaker cross-domain query. The prototype
  implements it as "compare `do_to_errc` results", which is wrong for the common
  same-domain case (it would claim two different codes that map to the same errc
  are equivalent). Correct policy:

```cpp
static bool equivalent(std::size_t a, error_domain_singleton const* other,
                       std::size_t b) noexcept {
    if (other == domain_singleton())       // same table
        return a == b;                     // code identity
    return to_errc(a) == other->do_to_errc(b);   // cross-domain via errc
}
```

## `do_name` — the fall-through bug

The prototype's switch falls through (no `break`), so every encoding ends up
with the last assignment. The rewrite should structure so that cannot happen:

```cpp
static void name(std::size_t, std::error_reporter_encoding encoding,
                 void* cookie, error_reporter_io_cookie_function cook) noexcept {
    static constexpr std::string_view utf8  = u8"win32";
    static constexpr std::u16string_view u16 = u"win32";
    static constexpr std::u32string_view u32 = U"win32";
    switch (encoding) {
    case error_reporter_encoding::utf16:   emit(u16, cookie, cook); return;
    case error_reporter_encoding::utf32:   emit(u32, cookie, cook); return;
    default:                               emit(utf8, cookie, cook); return;
    }
}
```

A shared `emit` helper takes the scatter list, so no per-encoding duplication
(see `imagined/05-reporting.md`).
