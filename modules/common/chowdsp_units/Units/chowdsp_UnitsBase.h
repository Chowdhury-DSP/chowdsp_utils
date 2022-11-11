#pragma once

namespace chowdsp::Units
{
/** Type representing an absolute unit. For example, an absolute unit of time might be "seconds" */
template <typename T>
struct AbsoluteUnit
{
    using value_type = T;
    using RelativeArgs = std::nullptr_t;
};

/** Type representing a relative unit. For example, a relative unit of time might be "samples" */
template <typename T, typename RelativeArgType>
struct RelativeUnit
{
    using value_type = T;
    using RelativeArgs = RelativeArgType;
};

/** Helper structs for metric-style units */
namespace Metric
{
    struct Peta
    {
        static constexpr auto multiplier = 1.0e15;
        static constexpr auto prefix = 'P';
    };

    struct Tera
    {
        static constexpr auto multiplier = 1.0e12;
        static constexpr auto prefix = 'T';
    };

    struct Giga
    {
        static constexpr auto multiplier = 1.0e9;
        static constexpr auto prefix = 'G';
    };

    struct Mega
    {
        static constexpr auto multiplier = 1.0e6;
        static constexpr auto prefix = 'M';
    };

    struct Kilo
    {
        static constexpr auto multiplier = 1.0e3;
        static constexpr auto prefix = 'k';
    };

    struct Unit
    {
        static constexpr auto multiplier = 1.0;
        static constexpr char prefix = {};
    };

    struct Milli
    {
        static constexpr auto multiplier = 1.0e-3;
        static constexpr auto prefix = 'm';
    };

    struct Micro
    {
        static constexpr auto multiplier = 1.0e-6;
        static constexpr char prefix = 'u'; // @TODO: how can we use 'μ' here instead?
    };

    struct Nano
    {
        static constexpr auto multiplier = 1.0e-9;
        static constexpr auto prefix = 'n';
    };

    struct Pico
    {
        static constexpr auto multiplier = 1.0e-12;
        static constexpr auto prefix = 'p';
    };

    struct Femto
    {
        static constexpr auto multiplier = 1.0e-15;
    };
} // namespace Metric

/** True if this unit type is an absolute unit */
template <typename UnitType>
constexpr bool IsAbsolute = std::is_base_of_v<AbsoluteUnit<typename UnitType::value_type>, UnitType>;

/** True if this unit type is a relative unit */
template <typename UnitType>
constexpr bool IsRelative = std::is_base_of_v<RelativeUnit<typename UnitType::value_type, typename UnitType::RelativeArgs>, UnitType>;

/** Converts between two unit types */
template <typename To, typename From>
constexpr std::enable_if_t<IsAbsolute<To> && IsAbsolute<From>, typename To::value_type>
    unit_cast (typename From::value_type sourceVal)
{
    return To::from_neutral (static_cast<typename To::value_type> (From::to_neutral (sourceVal)));
}

/** Converts between two unit types */
template <typename To, typename From>
constexpr std::enable_if_t<IsAbsolute<To> && IsRelative<From>, typename To::value_type>
    unit_cast (typename From::value_type sourceVal, const typename From::RelativeArgs& sourceRelArgs)
{
    return To::from_neutral (static_cast<typename To::value_type> (From::to_neutral (sourceVal, sourceRelArgs)));
}

/** Converts between two unit types */
template <typename To, typename From>
constexpr std::enable_if_t<IsRelative<To> && IsAbsolute<From>, typename To::value_type>
    unit_cast (typename From::value_type sourceVal, const typename To::RelativeArgs& destRelArgs)
{
    return To::from_neutral (static_cast<typename To::value_type> (From::to_neutral (sourceVal)), destRelArgs);
}
} // namespace chowdsp::Units
