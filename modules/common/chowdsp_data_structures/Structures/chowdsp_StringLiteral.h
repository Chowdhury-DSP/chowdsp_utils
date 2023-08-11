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
} // namespace sl_detail
#endif

/** A string-literal wrapper type. */
template <size_t N>
class StringLiteral
{
    std::array<char, N> chars {};
    size_t actual_size = 0;

    template <size_t NN, size_t MM>
    friend constexpr StringLiteral<NN + MM> operator+ (const StringLiteral<NN>&, const StringLiteral<MM>&);

public:
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
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_MSVC
