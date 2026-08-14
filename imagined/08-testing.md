# 08 · Testing without the herbception compiler wired in

Until Sema/codegen can fabricate `__CPP_STD_ERROR__` from `throw throws`, the
library still needs tests. The prototype's `pesudo_throws.h` throws a *dynamic*
C++ exception carrying a hand-built `{domain, code}` — good enough to exercise
`error_domain<T>`, the comparison operators, and the domain hooks, but it does
**not** test the `{T, i1}` payload flow or the discriminant.

## What `pesudo_throws` is actually good for

- constructing the two-word value through the real `error_domain<T>` bridge,
- the enum `operator==`, `do_equivalent`, `do_to_errc`,
- `do_name`/`do_message` rendering into a caller buffer,
- the `do_cleanup` hook firing exactly once per fabricated value.

These are pure library behaviors and are testable today.

## A better stand-in than `pesudo_throws.h`

Simulate the *payload* without the compiler by encoding the same ABI manually:

```cpp
struct throws_result {          // the {T, i1} payload the compiler would emit
    void*         domain;
    std::size_t   code;
    bool          failed;
};

throws_result call_failing() {
    return { error_domain<win32_errc>::domain(),
             error_domain<win32_errc>::code(win32_errc::file_not_found),
             true };
}

void consumer() {
    throws_result r = call_failing();
    if (r.failed) {
        // exactly what catch throws(std::error e) would bind
        std::error e;            // in the magic world: fabricated by the compiler
        // ...
    }
}
```

(For the prototype's `struct error`, binding is `std::error e{r.domain, r.code}`
— which is the same value the compiler would fabricate.)

## What only a real compile can verify

- `throw throws e;` fabrication and the discriminant store,
- `catch throws(std::error e)` reading the payload without landing pads,
- destructor-on-scope-exit of the catch variable,
- `noexcept` boundary enforcement (`catch throws` required, `try` prohibited),
- the register-level calling convention on each target.

Those belong in Clang lit tests (`clang/test/CodeGen/herbception-*.cpp`) once
the magic type exists. Until then the harness in this directory keeps the
library side honest and the compiler contract documented (`07-compiler-contract.md`).

## Suggested test matrix for the library

| Behavior | Check |
|----------|-------|
| size/align | `sizeof(std::error) == 16`, `alignof == 8` (LP64) |
| fabrication | `error_domain<T>::domain()/code()` round-trip |
| strict `==` | same domain+code → true; same code, other domain → false |
| `do_equivalent` | `win32 file_not_found` ≡ `errc no_such_file_or_directory` |
| `do_to_errc` | known + unknown codes (unknown keeps raw code) |
| `do_name` | each encoding returns non-empty, no fall-through |
| `do_message` | buffer sink receives expected bytes |
| `do_cleanup` | fires once; fires never for `nullptr` `do_cleanup` domains |
