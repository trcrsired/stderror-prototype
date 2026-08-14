# 07 · Compiler contract: how `std::error` binds to `throws`/`catch throws`

This is the *existing* compiler behavior — the design must fit it, not change it.

## The payload

`clang/lib/CodeGen/CGCall.cpp` (`getHerbceptionErrorType`) hardcodes the
implicit `throws` error payload as `{void*, size_t}`:

```llvm
; T foo() throws;
define { T, i1 } @foo(...) #throws {
  ; on error: %payload = { domain*, code }, %disc = true
}
```

`void foo() throws` returns `{ {void*, size_t}, i1 }`. `T foo() throws` returns
`{ union{T, error}, i1 }`. The magic builtin `__CPP_STD_ERROR__` *is* that
two-word `{domain*, code}` — by construction, no coercion needed.

## `throw throws e;`

Sema (`ActOnCXXThrowThrows`) already accepts an operand and `BuildCXXThrow`
stores it into the return slot with the discriminant set to true
(`CGStmt.cpp`). With a magic type:

- `throw throws std::win32_errc::file_not_found;`
- the enum is not the error itself. The compiler calls the enum→error bridge:
  `error_domain<win32_errc>::domain()` / `::code(e)`, fabricating the two-word
  value, then emits it into the payload slot and sets the discriminant.

So `error_domain<T>` is the **compiler-facing ABI bridge**. `pesudo_throws.h`
becomes unnecessary — the compiler does what that template did, but without a
dynamic exception.

## `catch throws(std::error e)`

The handler (`CGCall.cpp`, `HerbceptionCatchScopes`) loads the payload into the
catch variable's slot and branches to the handler. With the magic type:

- the catch variable's type is `__CPP_STD_ERROR__`;
- its destructor runs when the handler scope exits → `domain()->do_cleanup` if set;
- inside the handler, `e.domain()`, `e.code()`, `e.do_equivalent(T)`,
  `e.do_to_errc()`, `e.do_name(...)`, `e.do_message(...)` all work.

No personality function, no `__cxa_begin_catch`, no landing pad — the payload is
two registers and the discriminant is a branch.

## `fails{E}` is untouched

`T foo() fails{E}` uses the explicit `E` in the payload directly
(`EST_ThrowsTyped` → `ConvertType(E)`). `std::error` is only the implicit
`throws` error type. The two mechanisms coexist; `fails` never fabricates
`__CPP_STD_ERROR__`.

## What the magic type must guarantee the backend

1. `sizeof(__CPP_STD_ERROR__) == 2 words`, trivially copyable for ABI purposes,
   with no user-visible constructors.
2. It can be bitcast / coerced to `{void*, size_t}` with no codegen change.
3. It has a (trivial or domain-dispatched) destructor, so the catch variable
   lifetime works in the normal scope-exit path.
4. Its methods (`domain`, `code`, `do_equivalent`, `do_to_errc`, `do_name`,
   `do_message`) are emitted as ordinary member functions of a builtin type —
   nothing special for the backend, only for Sema/codegen construction.

## Summary of the binding

| Site | Today's prototype | Magic type design |
|------|-------------------|-------------------|
| Make an error | `error{error_domain<T>::domain(), error_domain<T>::code(e)}` (user code) | `throw throws e;` (compiler fabricates via `error_domain<T>`) |
| Carry it | `{void*, size_t}` payload (hardcoded) | same, but guaranteed by the type's definition |
| Receive it | `catch throws(std::error e)` reads payload | same |
| none (do_cleanup if set) | none | destructor → `domain()->do_cleanup` if set |
