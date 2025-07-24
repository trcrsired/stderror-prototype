#pragma once

template<typename T>
requires (::std::is_enum_v<T>)
inline constexpr void pesudo_throws(T x)
{
  using error_type = ::std::error_domain<decltype(x)>::error_type;
  throw ::std::error(error_type::domain(), error_type::code(x));
}
