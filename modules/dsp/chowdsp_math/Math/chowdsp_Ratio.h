#pragma once

namespace chowdsp
{
/**
 *  Compile-time rational expression.
 *
 *  This can be used as a template argument in cases where you might want to have a floating-point argument.
 */
template <std::intmax_t Num, std::intmax_t Den>
struct Ratio
{
    static_assert (Den != 0, "Ratio denominator must not be zero!");

    static constexpr auto numerator = Num;
    static constexpr auto denominator = Den;

    template <typename T>
    static constexpr std::enable_if_t<std::is_floating_point_v<T>, T> value = (T) Num / (T) Den;
};

/** Compile-time rational expression in scientific notation form. */
template <std::intmax_t Mantissa, std::intmax_t Exp, typename = void>
struct ScientificRatio;

#ifndef DOXYGEN
template <std::intmax_t Mantissa>
struct ScientificRatio<Mantissa, 0> : Ratio<Mantissa, 1>
{
};

template <std::intmax_t Mantissa, std::intmax_t Exp>
struct ScientificRatio<Mantissa, Exp, std::enable_if_t<(Exp < 0)>> : Ratio<Mantissa, gcem::pow<std::intmax_t, std::intmax_t> (10, -Exp)>
{
};

template <std::intmax_t Mantissa, std::intmax_t Exp>
struct ScientificRatio<Mantissa, Exp, std::enable_if_t<(Exp > 0)>> : Ratio<Mantissa * gcem::pow<std::intmax_t, std::intmax_t> (10, Exp), 1>
{
};
#endif
} // namespace chowdsp
