#pragma once

namespace chowdsp
{
/** Helpers for working with std::string_view */
namespace StringViewHelpers
{
    /**
     * Joins multiple string views together into a single new string view.
     *
     * Implementation adapted from: https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time
     */
    template <std::string_view const&... Strs>
    struct join
    {
        // Join all strings into a single std::array of chars
        static constexpr auto impl() noexcept
        {
            constexpr std::size_t len = (Strs.size() + ... + 0);
            std::array<char, len + 1> local_arr {};
            auto append = [i = (size_t) 0, &local_arr] (auto const& s) mutable
            {
                for (auto c : s)
                    local_arr[i++] = c;
            };
            (append (Strs), ...);
            local_arr[len] = 0;
            return local_arr;
        }
        // Give the joined string static storage
        static constexpr auto arr = impl();
        // View as a std::string_view
        static constexpr std::string_view value { arr.data(), arr.size() - 1 };
    };

    // Helper to get the value out
    template <std::string_view const&... Strs>
    static constexpr auto join_v = join<Strs...>::value;
} // namespace StringViewHelpers
} // namespace chowdsp
