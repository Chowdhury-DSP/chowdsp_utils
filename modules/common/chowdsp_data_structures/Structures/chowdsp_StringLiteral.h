#pragma once

#include <array>
#include <string>
#include <string_view>

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4820)

namespace chowdsp
{
#ifndef DOXYGEN
namespace sl_detail
{
#if (defined(__cplusplus) && __cplusplus >= 202002L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
    using std::copy;
#else
    template <class InputIt, class OutputIt>
    constexpr OutputIt copy (InputIt first, InputIt last, OutputIt d_first)
    {
        for (; first != last; (void) ++first, (void) ++d_first)
            *d_first = *first;
        return d_first;
    }
#endif

    /** Counts the string length needed to contain a number. */
    template <typename Int>
    constexpr size_t num_str_len (Int number)
    {
        size_t digits = (number > 0) ? 0 : 1;
        while (number)
        {
            number /= 10;
            digits++;
        }
        return digits;
    }

    constexpr void uint_to_str (char* str, size_t size, uint64_t value)
    {
        size_t i = size;
        while (i > 0)
        {
            str[i - 1] = static_cast<char> ('0' + (value % 10));
            value /= 10;
            i--;
        }
    }

    constexpr void sint_to_str (char* str, size_t size, int64_t value)
    {
        if (value >= 0)
            return uint_to_str (str, size, static_cast<uint64_t> (value));

        str[0] = '-';
        uint_to_str (str + 1, size - 1, static_cast<uint64_t> (-value));
    }
} // namespace sl_detail
#endif

/** A string-literal wrapper type. */
template <size_t N>
struct StringLiteral
{
    std::array<char, N> chars {};
    size_t actual_size = 0;

    constexpr StringLiteral() = default;
    constexpr StringLiteral (const StringLiteral&) = default;
    constexpr StringLiteral& operator= (const StringLiteral&) = default;
    constexpr StringLiteral (StringLiteral&&) noexcept = default;
    constexpr StringLiteral& operator= (StringLiteral&&) noexcept = default;

    explicit constexpr StringLiteral (char letter)
        : StringLiteral (&letter, std::integral_constant<size_t, 1> {})
    {
    }

    constexpr StringLiteral (const char (&str)[N]) // NOSONAR NOLINT(google-explicit-constructor)
        : StringLiteral (str, std::integral_constant<size_t, N - 1> {})
    {
    }

    template <size_t M>
    constexpr StringLiteral (const char* c, std::integral_constant<size_t, M> num)
        : actual_size (num)
    {
        static_assert (num <= N);
        sl_detail::copy (c, c + actual_size, chars.begin());
    }

    [[nodiscard]] constexpr std::string_view toStringView() const { return { data(), size() }; }

    template <typename IntType, typename = typename std::enable_if_t<std::is_integral_v<IntType>>>
    constexpr explicit StringLiteral (IntType int_value)
        : actual_size (sl_detail::num_str_len (int_value))
    {
        // N is not large enough to hold this number!
#if __cplusplus >= 202002L || _MSVC_LANG >= 202002L
        jassert (N >= actual_size);
#endif

        if constexpr (std::is_signed_v<IntType>)
            sl_detail::sint_to_str (chars.data(), actual_size, static_cast<int64_t> (int_value));
        else
            sl_detail::uint_to_str (chars.data(), actual_size, static_cast<uint64_t> (int_value));
    }
    constexpr operator std::string_view() const { return toStringView(); } // NOSONAR NOLINT(google-explicit-constructor)
    [[nodiscard]] std::string toString() const { return { data(), size() }; }
    operator std::string() const { return toString(); } // NOSONAR NOLINT(google-explicit-constructor)
#if CHOWDSP_USING_JUCE
    [[nodiscard]] juce::String toJUCEString() const
    {
        return toString();
    }
    operator juce::String() const { return toJUCEString(); } // NOSONAR NOLINT(google-explicit-constructor)
#endif

    [[nodiscard]] constexpr const char* data() const
    {
        return chars.data();
    }
    [[nodiscard]] constexpr size_t size() const { return actual_size; }
    constexpr auto begin() const { return chars.begin(); }
    constexpr auto end() const { return chars.begin() + actual_size; }
};

template <size_t N>
std::ostream& operator<< (std::ostream& os, const StringLiteral<N>& sl)
{
    os << std::string_view (sl.data(), sl.size());
    return os;
}

template <size_t N, size_t M>
constexpr StringLiteral<N + M> operator+ (const StringLiteral<N>& sl1, const StringLiteral<M>& sl2)
{
    StringLiteral<N + M> result;
    sl_detail::copy (sl1.begin(), sl1.end(), result.chars.data());
    sl_detail::copy (sl2.begin(), sl2.end(), result.chars.data() + sl1.size());
    result.actual_size = sl1.actual_size + sl2.actual_size;
    return result;
}

template <size_t N, size_t M>
constexpr StringLiteral<N + M> operator+ (const StringLiteral<N>& sl1, const char (&chars)[M])
{
    return sl1 + StringLiteral { chars };
}

template <size_t N, size_t M>
constexpr bool operator== (const StringLiteral<N>& lhs, const StringLiteral<M>& rhs) noexcept
{
    return lhs.toStringView() == rhs.toStringView();
}

template <size_t N, size_t M>
constexpr bool operator== (const StringLiteral<N>& lhs, const char (&rhs)[M]) noexcept
{
    return lhs == StringLiteral<M> { rhs };
}

template <size_t N, size_t M>
constexpr bool operator== (const char (&lhs)[N], const StringLiteral<M>& rhs) noexcept
{
    return StringLiteral<N> { lhs } == rhs;
}

template <size_t N>
constexpr bool operator== (const std::string_view& lhs, const StringLiteral<N>& rhs) noexcept
{
    return lhs == rhs.toStringView();
}

template <size_t N, size_t M>
constexpr bool operator!= (const StringLiteral<N>& lhs, const StringLiteral<M>& rhs) noexcept
{
    return ! (lhs == rhs);
}

template <size_t N, size_t M>
constexpr bool operator!= (const StringLiteral<N>& lhs, const char (&rhs)[M]) noexcept
{
    return ! (lhs == rhs);
}

template <size_t N, size_t M>
constexpr bool operator!= (const char (&lhs)[N], const StringLiteral<M>& rhs) noexcept
{
    return ! (lhs == rhs);
}

template <size_t N>
constexpr bool operator!= (const std::string_view& lhs, const StringLiteral<N>& rhs) noexcept
{
    return ! (lhs == rhs);
}

#if (defined(__cplusplus) && __cplusplus >= 202002L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
namespace string_literals
{
    template <StringLiteral sl>
    constexpr auto operator"" _sl()
    {
        return sl;
    }

    template <char... str>
    constexpr auto operator"" _sl_n()
    {
        constexpr char str_array[] { str..., '\0' };
        return StringLiteral { str_array };
    }
} // namespace string_literals
#endif
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_MSVC
