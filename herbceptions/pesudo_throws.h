#pragma once

// This template is the prototype's stand-in for the compiler's synthetic
// `throw throws e;`. Only the compiler (here: this function) may manufacture
// a std::error; it does so by going through error_domain<T> and calling its
// domain() and code() functions, exactly like the real compiler will.
//
// User code must never construct std::error itself: its default/copy/move
// constructors are deleted and all data members are private.
template<typename T>
requires (::std::is_class_v<T> || ::std::is_enum_v<T>)
inline constexpr void pesudo_throws(T x)
{
  using error_domain_type = ::std::error_domain<T>;
  throw ::std::error{error_domain_type::domain(), error_domain_type::code(x)};
}
