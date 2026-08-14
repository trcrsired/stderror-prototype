# 05 · IO-based reporting: `do_name` / `do_message`

The whole reporting model must be **allocation-free and string-free**, because
`std::error` is a two-register value that cannot own memory and the feature must
stay freestanding-friendly. The prototype already has the right skeleton
(`error_reporter_encoding`, `io_scatter_t`,
`error_reporter_io_cookie_function`) — this doc sharpens it.

## The cookie / scatter contract

```cpp
struct io_scatter_t {
    void const* base;
    std::size_t len;
};

enum class error_reporter_encoding {
    utf8, utf16, utf32, gb18030, utfebcdic
};

// Receives one or more scatter segments. The domain's do_name/do_message
// never allocate; they hand bytes to this callback.
using error_reporter_io_cookie_function =
    void (*)(std::error_reporter_encoding encoding,
             void* cookie,
             io_scatter_t const* segs,
             std::size_t count) noexcept;
```

`cookie` is an opaque pointer supplied by the caller — a custom IO writer, a
`fast_io` stream, a fixed buffer, whatever. The domain only produces bytes.

## Why IO-based and not `const char*` / `std::string`

1. **No ownership.** `error` has no constructor and no storage; it cannot hold a
   heap string. A `const char*` message would force the domain to leak or the
   error to own — both impossible.
2. **No encoding dependency.** `strerror` gives the locale's narrow bytes;
   Windows wants UTF-16. The encoding parameter lets the caller say what it can
   consume, and the domain writes in that encoding.
3. **Freestanding.** No `<string>`, no `new`, no codecvt tables in the core.
4. **Caller-controlled storage.** The same `error` can be rendered into a
   stack buffer, a log line, or directly to a socket.

## Caller side

```cpp
void report(std::error e) noexcept {
    // Render directly into a caller-owned buffer (UTF-8).
    char buf[256];
    std::size_t used = 0;
    error_reporter_io_cookie_function sink =
        [](std::error_reporter_encoding, void* c, io_scatter_t const* segs, std::size_t n) noexcept {
            auto* s = static_cast<BufferSink*>(c);
            for (std::size_t i = 0; i != n; ++i) s->append(segs[i].base, segs[i].len);
        };
    BufferSink sink_cookie{ buf, sizeof buf, used };
    e.do_message(error_reporter_encoding::utf8, &sink_cookie, sink);
}
```

## Domain side

```cpp
static void message(std::size_t code, std::error_reporter_encoding enc,
                    void* cookie, error_reporter_io_cookie_function cook) noexcept {
    switch (enc) {
    case error_reporter_encoding::utf16: {
        io_scatter_t v; v.base = u"file not found"; v.len = sizeof(u"file not found") - sizeof(char16_t);
        cook(enc, cookie, &v, 1);
        return;
    }
    default: {
        io_scatter_t v; v.base = u8"file not found"; v.len = sizeof(u8"file not found") - 1;
        cook(enc, cookie, &v, 1);
        return;
    }
    }
}
```

Real domains look up a message table keyed by code and write the appropriate
segment. A table of `{code, u8, u16, u32}` literals covers the common encodings
in one place.

## `do_name` vs `do_message`

- `do_name` → the domain's short name ("win32", "nt", "com", "posix", "wine").
- `do_message` → the human message for the specific code.

Both use the same cookie/scatter contract; the only difference is the content
produced.

## Naming/encoding edge cases

- `gb18030` / `utfebcdic`: provide where a target genuinely needs them; for
  domains without a native representation, fall back to UTF-8 (a domain is
  allowed to answer a different encoding than requested — the caller must be
  prepared for that, e.g. default to utf8 if the encoding is unsupported).
- The caller, not the domain, decides where bytes go — so partial writes are
  the caller's problem (it can count bytes via its cookie).
