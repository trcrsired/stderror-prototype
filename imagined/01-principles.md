# 01 · Principles: what is wrong, and what must stay true

## What the current prototype gets right

1. **Two-word layout.** `error { domain, code }` is 16 bytes / 2 registers on
   x86-64 — exactly what the compiler's `{void*, size_t}` payload expects.
2. **`error_domain_singleton` as a plain function-pointer table** (a "C vtable"),
   not a C++ vtable. No RTTI, no vptr, `constinit`-able, C-linkable.
3. **`error_domain<T>` as the customization point** keyed on an enum type, with
   `domain()` and `code()` static functions.
4. **IO-based reporting** via `error_reporter_io_cookie_function` + `io_scatter_t`,
   so `do_name`/`do_message` never allocate or build `std::string`.
5. **Weak, overridable `__cxa_error_domain_*` accessors**, so a freestanding or
   Wine environment can substitute its own domain singleton.
6. **`do_to_errc()`** as a portable escape hatch to the existing `std::errc`
   world (`strerror`, `std::error_code` interop).

## The core mistake: `error` as a library struct

The prototype defines `std::error` as a normal struct. That drags in a whole
dependency surface the feature is supposed to avoid:

- constructors (default / copy / move) that users can call,
- an ABI that library and compiler must agree on forever,
- copy semantics that must be tediously deleted once cleanup exists,
- a construction path (`error{domain, code}`) that lets users smuggle values in.

**Fix: make `std::error` a compiler magic builtin (`__CPP_STD_ERROR__`), named
by the library as `namespace std { using error = __CPP_STD_ERROR__; }`.** It has
*no constructors at all* — not default, not copy, not move. It has `.domain()`
and `.code()`. It can only be manufactured by `throw throws e;` and consumed by
`catch throws(std::error e){}`. The destructor checks `domain()->do_cleanup` and
calls it if non-null.

This forces the type to be handled as a **trivial two-register struct**:
- no copy/move ctor calls anywhere — the compiler just moves two words,
- no way to misconstruct, mis-copy, or store it,
- layout guaranteed by the compiler, not by a library that can drift,
- `{ptr, size_t}` payload flow is definitional.

## What is bad / missing in the prototype (beyond the struct mistake)

### win32_domain.cc
- **The `do_name` switch falls through**: every case assigns `v.base`/`v.len`
  and then falls into the next case, so all encodings resolve to the last
  assignment. (No `break`.)
- Only 3 `win32_errc` values — a real library needs the full documented set.
- `do_to_errc` uses `default: invalid_argument` — loses information where no
  errc exists; should preserve the raw code.

### Domain table
- **No `do_cleanup` hook** — needed for the magic-type destructor to own anything.
- Several hooks default to `nullptr`; the domain pointer itself is never null
  (contract), so only the hooks need null-checking.
- `do_equivalent` is "compare `do_to_errc` results", which loses code identity
  for the common same-domain case.

### Reporting / surface
- `do_throw_dynamic_exception` is a bad idea — reintroduces the traditional EH
  path the feature is meant to remove. Drop it (already `#if 0`-ed).

## Design constraints (must always hold)

1. **Two words, nothing more.** `{ void* domain; size_t code; }`. No hidden data.
2. **`error` is magic-builtin and trivial-by-construction.** No constructors,
   no copy/move, no storage.
3. **No heap allocation** for the common path (encoding, naming, messaging).
4. **`constinit`-able domain singletons.** No dynamic init, no destruction-order
   hazards.
5. **C ABI interop for the domain table** (`extern "C"` + plain function
   pointers), so any language can implement a domain.
6. **Freestanding-friendly** — no `std::string`, no `new`, no `strerror`
   dependency inside the core header.
7. **Layout-locked to the compiler contract** (`imagined/07-compiler-contract.md`).

## Proposed fixes in one line each

- Make `std::error` a magic builtin; library only provides the `using`.
- `.domain()` / `.code()` only; add `do_cleanup` hook on the singleton.
- Keep enum-based `operator==`; no `error`-to-`error` user comparison needed.
- Enumerate full `win32_errc` / `nt_errc` / `com_errc` / `wine_errc` / `errc` sets.
- Rewrite `do_name` to use a helper that cannot fall through.
- Delete `do_throw_dynamic_exception`.
