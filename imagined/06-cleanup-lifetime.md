# 06 · Cleanup and lifetime: the `do_cleanup` hook

This is the one place the magic type's destructor does real work. Everything
here follows from one rule: **the error is fabricated by the compiler, flows as
two registers, and is materialized exactly once** — so ownership can never be
duplicated and cleanup runs exactly once.

## When is `do_cleanup` non-null?

`std::error` is a two-register value. Stateless domains (POSIX errno, Win32,
NTSTATUS, HRESULT) map a code to a *static* message table — nothing to free.
`do_cleanup` stays `nullptr` and the destructor is a no-op: zero cost.

A domain sets `do_cleanup` only when it must own something the error itself cannot
hold (the error has no heap storage), e.g.:

- a lazily-built, per-code message string that was allocated once and cached,
- an OS resource handle that should be released when the catch variable dies,
- anything else that must run "at end of scope" for this specific error value.

## The destructor, exactly

```cpp
// Compiler-generated:
~error() noexcept {
    // domain() is never null by contract; only do_cleanup is optional.
    if (auto* d = this->domain(); d->do_cleanup)
        d->do_cleanup(d, this->code());
}
```

Because the magic type has **no copy/move constructors**, there is no path that
produces two live copies of the same `{domain, code}`. Every catch variable is
the unique owner of its payload. `do_cleanup` is therefore called exactly once.

## Why the register-flow is safe

The value is created by `throw throws e;` (two registers), stored into the
`{T, i1}` payload slot, returned, unpacked, and bound to the catch parameter.
At no point does the language run a copy/move ctor (there are none). The
destructor runs only when the catch variable's scope ends. Compare with a
library struct, where every copy would have to be manually deleted and every
move would have to nullify — and where the compiler's `{void*, size_t}` payload
coercion would silently bypass those safeguards.

## Guarantees to document

1. `domain()` is **never null** — the compiler fabricates `{domain, code}` with
   a valid domain pointer, always. No null-check on the domain in any code path.
2. `do_cleanup` is invoked at most once per fabricated error.
3. `do_cleanup` is `noexcept` and must not throw (called from a destructor).
4. A `nullptr` `do_cleanup` means "pure value; destructor is a no-op".
5. `do_cleanup` must be idempotent-safe for a *given* `{domain, code}` only in the
   sense that the domain controls when/how many times a code may be finalized —
   the magic type guarantees a single call per value.
6. `do_to_errc`, `do_equivalent`, `do_name`, `do_message` must remain usable
   on a code that has been cleaned up only if the domain says so; by default the
   do_cleanup hook should not make the code unqueryable.

## Design decision: keep it nullable, not mandatory

`do_cleanup` is a hook, not a requirement. Forcing every domain to implement a
no-op function pointer would add a pointless call on every catch. Null-check
cost is one branch; a null function pointer is the zero-cost fast path.
