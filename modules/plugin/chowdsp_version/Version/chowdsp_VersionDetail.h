#pragma once

#include <string_view>

namespace chowdsp
{
#ifndef DOXYGEN
namespace version_detail
{
    /** Borrowed from: https://stackoverflow.com/questions/25195176/how-do-i-convert-a-c-string-to-a-int-at-compile-time */
    constexpr int stoi (std::string_view str, std::size_t* pos = nullptr)
    {
        using namespace std::literals;
        const auto numbers = "0123456789"sv;

        const auto begin = str.find_first_of (numbers);
        if (begin == std::string_view::npos)
            throw std::invalid_argument { "stoi" };

        const auto sign = begin != 0U && str[begin - 1U] == '-' ? -1 : 1;
        str.remove_prefix (begin);

        const auto end = str.find_first_not_of (numbers);
        if (end != std::string_view::npos)
            str.remove_suffix (std::size (str) - end);

        auto result = 0;
        auto multiplier = 1;
        for (auto i = static_cast<ptrdiff_t> (std::size (str) - 1U); i >= 0; --i)
        {
            auto number = (int) *(str.data() + i) - '0';
            result += number * multiplier * sign;
            multiplier *= 10;
        }

        if (pos != nullptr)
            *pos = begin + std::size (str);
        return result;
    }
} // namespace version_detail
#endif // DOXYGEN
} // namespace chowdsp