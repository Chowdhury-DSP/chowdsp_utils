#pragma once

namespace chowdsp
{
/** Helper functions for working with the nameof library */
namespace NameOfHelpers
{
    /**
     * Returns the name of a type without any scope prefixes.
     *
     * For example, if NAMEOF (T) returns "foo::bar::MyTypeName", then
     * getLocalTypeName<T>() will return "MyTypeName".
     */
    template <typename T>
    constexpr std::string_view getLocalTypeName() noexcept
    {
        constexpr auto typeName = NAMEOF_TYPE (T);
        constexpr auto endOfLastScopeName = typeName.find_last_of ("::");
        return endOfLastScopeName == std::string_view ::npos
                   ? typeName
                   : typeName.substr (std::min (endOfLastScopeName + 1, typeName.size()));
    }
} // namespace NameOfHelpers
} // namespace chowdsp
