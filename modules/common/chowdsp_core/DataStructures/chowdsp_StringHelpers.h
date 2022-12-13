#pragma once

namespace chowdsp
{
#if CHOWDSP_USING_JUCE
/** Converts a std::string_view to a juce::String */
inline juce::String toString (const std::string_view& sv) noexcept
{
    return juce::String { sv.data(), sv.size() };
}
#endif

#if __cplusplus >= 202002L
#ifndef DOXYGEN
namespace string_helpers_detail
{
    // Implementation details borrowed from: https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time
    template <const std::string_view&... Strs>
    struct concat
    {
        // Join all strings into a single std::array of chars
        static constexpr auto impl() noexcept
        {
            constexpr size_t len = (Strs.size() + ... + 0);
            std::array<char, len + 1> arr_internal {};
            auto append = [i = (size_t) 0, &arr_internal] (auto const& s) mutable
            {
                for (auto c : s)
                    arr_internal[i++] = c;
            };
            (append (Strs), ...);
            arr_internal[len] = 0;
            return arr_internal;
        }
        // Give the joined string static storage
        static constexpr auto arr = impl();
        // View as a std::string_view
        static constexpr std::string_view value { arr.data(), arr.size() - 1 };
    };
} // namespace string_helpers_detail
#endif // DOXYGEN

/** Concatenates a group of string_views */
template <std::string_view const&... Strs>
static constexpr auto string_view_concat = string_helpers_detail::concat<Strs...>::value;
#endif // __cplusplus >= 202002L
} // namespace chowdsp
