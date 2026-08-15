#include<type_traits>
#include<cstddef>
#include<system_error>
#include<cstdint>
#include"error.h"

namespace my_company
{

enum class win32_errc :
    ::std::uint_least32_t
{
    success=0,
    invalid_function=1,
    file_not_found=2
};

}

namespace std
{

template<>
class error_domain<::my_company::win32_errc>
{
public:
    using errc_type = my_company::win32_errc;
    using domain_alias_type = ::std::error_domain<::std::win32_errc>;   //compiler will know they are under same category. no need category any more
    static inline constexpr ::std::size_t code(errc_type e) noexcept
    {
        return static_cast<::std::size_t>(static_cast<::std::uint_least32_t>(e));
    }
};

}
