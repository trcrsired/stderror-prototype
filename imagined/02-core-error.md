# 02 · `std::error` is a magic builtin type

`std::error` should **not** be a library struct. It is a **compiler magic type**,
in the same family as `__SIZE_TYPE__`, `__PTRDIFF_TYPE__`, etc. The standard
library only provides the name:

```cpp
namespace std { using error = __CPP_STD_ERROR__; }
```

The compiler owns the type: its layout, its lifetime, and its only two
manufacturing sites. User code cannot construct it, copy it, move it, or
store it — it exists only as a parameter of `catch throws`.

## Why magic beats a library struct

| | Library `struct error` | Magic builtin `__CPP_STD_ERROR__` |
|---|---|---|
| Construction sites | anywhere (accidentally) | only `throw throws` / the payload |
| Copy/move | must be carefully deleted | **no constructors at all, not even copy/move** — impossible to misuse |
| ABI drift | library and compiler must agree forever | one source of truth: the compiler |
| Payload fit | must be manually kept at `{ptr, size_t}` | guaranteed by definition |
| `catch throws` binding | type must match compiler's guess | the compiler knows its own type |
| User error values | can be smuggled via `error{...}` | **impossible** — only `throw throws e` |
| Cleanup | `do_cleanup`, must be added and kept in sync | `do_cleanup` hook, wired by the compiler's destructor |

## What the type looks like to the user

```cpp
namespace std {
    using error = __CPP_STD_ERROR__;
}

// The only legal ways an `error` value can appear:

void f() throws {
    throw throws std::win32_errc::file_not_found;   // (1) manufacture
}

void g() noexcept {
    try {
        f();
    }
    catch throws(std::error e) {                    // (2) consume
        auto d = e.domain();                        // error_domain_singleton const*
        auto c = e.code();                          // std::size_t
    }
}
```

That's it. No `error e;`, no `error e = ...;`, no `return error{...}`,
no copying, no storing in containers.

## Member surface

The type has **no constructors** (default, copy, and move all absent). It has
read-only accessors that dispatch into the domain:

```cpp
constexpr error_domain_singleton const* domain() const noexcept;  // the table
constexpr std::size_t                     code() const noexcept;  // the code

// Cross-domain equivalence: "does *this mean the same thing as enum t?"
// Distinct from operator== (which requires identical domain AND code).
template <typename T>
requires std::is_enum_v<T>
constexpr bool do_equivalent(T t) const noexcept;

// Portable mapping to std::errc (for strerror / error_code interop).
constexpr std::errc do_to_errc() const noexcept;
```

`do_equivalent` dispatches into **this error's** domain:
`domain()->do_equivalent(code, error_domain<T>::domain(), error_domain<T>::code(t))`.
`do_to_errc` dispatches to `domain()->do_to_errc(code)`.

And a destructor, whose semantics are the interesting part:

```cpp
// Compiler-generated, conceptually:
~error() noexcept {
    // domain() is never null by contract; only do_cleanup is optional.
    if (auto* d = domain(); d->do_cleanup)
        d->do_cleanup(d, code());
}
```

`do_cleanup` is a nullable hook on `error_domain_singleton`. The common case
(POSIX/Win32/NT/COM codes) leaves it `nullptr` — the destructor is a no-op and
the error is a pure two-register value. Domains that own resources (e.g. a
lazily-built message buffer) set `do_cleanup`; the compiler calls it exactly once,
when the `catch throws` parameter goes out of scope.

## How `throw throws e` manufactures one

`throw throws e` where `e` is a `*_errc` enum compiles to code roughly like:

```cpp
__CPP_STD_ERROR__ __builtin_error_from_domain(
    error_domain<T>::domain(), error_domain<T>::code(e));
```

The enum → `{domain, code}` mapping is exactly what the prototype's
`error_domain<T>` already provides (`domain()` + `code()`); it becomes the
**compiler-facing ABI bridge** rather than a user-facing construction path.
`pesudo_throws.h` is then unnecessary — the compiler does it.

## Comparison

**`operator==` is strict identity, not equivalence.** Both the domain pointer
*and* the code must match. Because there are no constructors, comparison is
between an error and a concrete enum. The comparison is delegated to **the
enum's domain** via its `pass` method — the enum value is passed in along with
the other side's `(domain, code)`:

```cpp
template <typename T> requires std::is_enum_v<T>
constexpr bool operator==(std::error e, T t) noexcept {
    // win32_errc::xxx → win32 domain's pass(win32_errc::xxx, e.domain(), e.code())
    return error_domain<T>::domain()->pass(t, e.domain(), e.code());
}
```

So `e == std::win32_errc::file_not_found` calls
`win32_domain->pass(win32_errc::file_not_found, e.domain(), e.code())`, and the
domain decides strict identity (same domain + same code). The symmetric
`T == std::error` delegates to the same call.

There is no `operator==(error, error)` in user code: two errors can only be
compared where one side is a concrete enum.

**`do_equivalent(T)` is a different, weaker concept.** It answers "does this
error mean the same thing as enum value `t`, possibly from another domain?"
(e.g. `win32_errc::file_not_found` vs `errc::no_such_file_or_directory`). It is
a method on the error that dispatches into **this error's** domain's
`do_equivalent` hook, and it is never folded into `==` — strict identity is
never silently relaxed.

## Layout guarantee

The compiler defines `__CPP_STD_ERROR__` as exactly two words
(`{ptr, size_t}`, 16 bytes on LP64) so it flows through the `{T, i1}` payload
and the register-discriminant conventions unchanged
(see `07-compiler-contract.md`). Nothing in the library can drift.
