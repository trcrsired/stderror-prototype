#pragma once

template<typename T>
requires (::std::is_enum_v<T>)
inline constexpr void pesudo_throws(T x)
{
  using error_domain_type = ::std::error_domain<T>;
  throw ::std::error{error_domain_type::domain(), error_domain_type::code(x)};
}
