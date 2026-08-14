# Imagined: a real `std::error` design

Design notes for turning the `stderror-prototype` into a real, production-grade
`std::error` that matches the `throws`/`fails{E}` compiler contract.

These are **ideas only**. No compiler code and no prototype code is changed.

## How to read this

| File | Topic |
|------|-------|
| `01-principles.md` | What is wrong with the current prototype; the design constraints we must respect |
| `02-core-error.md` | The `std::error` value type itself (layout, constructors, comparison, lifetime) |
| `03-error-domain.md` | `error_domain_singleton` (the function-pointer table) and `error_domain<T>` customization point |
| `04-standard-domains.md` | POSIX / Win32 / NT / COM / Wine domains and enum coverage |
| `05-reporting.md` | IO-based `do_name`/`do_message` reporting, encodings, no heap |
| `06-cleanup-lifetime.md` | the `do_cleanup` hook, lifetime, and why the register flow makes it safe |
| `07-compiler-contract.md` | How the `{T, i1}` payload, `throw throws`, and `catch throws` bind to `std::error` |
| `08-testing.md` | How to validate the library without the herbception compiler wired in |

## The one hard constraint

The Clang backend hardcodes the implicit `throws` error payload as `{void*, size_t}`
(`clang/lib/CodeGen/CGCall.cpp`, `getHerbceptionErrorType`). The `catch throws(std::error e)`
handler loads the payload straight into a `std::error` alloca, and `throw throws`
stores the thrown value into that slot. Therefore:

> **`std::error` must be exactly two words: `{ error_domain_singleton const* domain; size_t code; }`.**
> It is a compiler magic builtin (`__CPP_STD_ERROR__`), so this is guaranteed by
> construction and cannot drift.

`fails{E}` is unaffected because it uses the explicit error type `E` directly.
