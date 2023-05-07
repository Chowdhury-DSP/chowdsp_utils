#pragma once

namespace chowdsp::Units
{
/** Represents an absolute unit of time in metric units (i.e. seconds, milliseconds, etc) */
template <typename Scale = Metric::Unit, typename T = float>
struct SecondsUnit : AbsoluteUnit<T>
{
    /** Converts this value to a neutral unit (in this case seconds) */
    static constexpr T to_neutral (T val)
    {
        return val * (T) Scale::multiplier;
    }

    /** Obtains this value from a neutral unit (in this case seconds) */
    static constexpr T from_neutral (T val)
    {
        return val / (T) Scale::multiplier;
    }

    static constexpr auto prefix { Scale::prefix };
    static constexpr std::string_view name = "s";
};

using Seconds = SecondsUnit<>;
using MilliSeconds = SecondsUnit<Metric::Milli>;
using MicroSeconds = SecondsUnit<Metric::Micro>;
using NanoSeconds = SecondsUnit<Metric::Nano>;

/** Represents a relative unit of time in samples */
template <typename T = float, typename FSType = std::conditional_t<std::is_floating_point_v<T>, T, double>>
struct SamplesUnit : RelativeUnit<T, FSType>
{
    using SampleRateType = FSType;

    /** Converts this value to a neutral unit (in this case seconds) */
    static constexpr T to_neutral (T val, SampleRateType sampleRate)
    {
        return static_cast<T> (static_cast<SampleRateType> (val) / sampleRate);
    }

    /** Obtains this value from a neutral unit (in this case seconds) */
    static constexpr T from_neutral (T val, SampleRateType sampleRate)
    {
        return static_cast<T> (static_cast<SampleRateType> (val) * sampleRate);
    }

    static constexpr std::string_view prefix {};
    static constexpr std::string_view name = "samples";
};

using Samples = SamplesUnit<>;

/** Units class for working with time. */
template <typename Unit>
class Time;

/** Time specialization for seconds-style units */
template <typename Scale>
class Time<SecondsUnit<Scale>>
{
    using Unit = SecondsUnit<Scale>;
    using T = typename Unit::value_type;

public:
    Time() = default;

    /** Constructs a unit of time from a base value */
    constexpr Time (T initialValue) // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here
        : val (initialValue)
    {
    }

    /** Constructs a unit of time from some other absolute time unit */
    template <typename OtherUnit, typename = typename std::enable_if_t<IsAbsolute<OtherUnit>>>
    constexpr Time (const Time<OtherUnit>& other) // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here
        : val (unit_cast<Unit, OtherUnit> (other.value()))
    {
    }

    /** Constructs a unit of time from some relative time unit */
    template <typename OtherUnit>
    constexpr Time (const Time<OtherUnit>& other, typename std::enable_if_t<IsRelative<OtherUnit>>* = nullptr) // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here
        : val (unit_cast<Unit, OtherUnit> (other.value(), other.getRelativeArgs()))
    {
    }

    /** Returns the time value */
    [[nodiscard]] constexpr auto value() const noexcept { return val; }

    /** Returns the time value */
    constexpr operator T() const noexcept { return val; } // NOLINT(google-explicit-constructor)

private:
    T val {};
};

template <typename T>
class Time<SamplesUnit<T>>
{
    using Unit = SamplesUnit<T>;
    using SampleRateType = typename Unit::SampleRateType;

public:
    Time() = default;

    /** Constructs a unit of time from a number of samples and sample rate */
    constexpr Time (T initialValue, SampleRateType sampleRate) // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here
        : val (initialValue),
          fs (sampleRate)
    {
    }

    /** Constructs a unit of time from an absolute unit of time and sample rate */
    template <typename OtherUnit, typename = typename std::enable_if_t<IsAbsolute<OtherUnit>>>
    constexpr Time (const Time<OtherUnit>& other, SampleRateType sampleRate) // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here
        : val (unit_cast<Unit, OtherUnit> (other.value(), sampleRate)),
          fs (sampleRate)
    {
    }

    /** Returns the time value */
    [[nodiscard]] constexpr auto value() const noexcept { return val; }

    /** Returns the time value */
    constexpr operator T() const noexcept { return val; } // NOLINT(google-explicit-constructor)

private:
    template <typename>
    friend class Time;

    [[nodiscard]] constexpr auto getRelativeArgs() const noexcept { return fs; }

    T val {};
    const SampleRateType fs;
};

template <typename Unit>
std::ostream& operator<< (std::ostream& os, const Time<Unit>& time)
{
    return os << time.value() << " " << Unit::prefix << Unit::name;
}

template <typename Unit, typename OtherUnit>
constexpr bool operator== (const Time<Unit>& lhs, const Time<OtherUnit>& rhs)
{
    return lhs.value() == Time<Unit> { rhs }.value();
}

template <typename Unit, typename OtherUnit>
constexpr bool operator!= (const Time<Unit>& lhs, const Time<OtherUnit>& rhs)
{
    return ! (lhs == rhs);
}

template <typename Unit, typename OtherUnit>
constexpr bool operator<(const Time<Unit>& lhs, const Time<OtherUnit>& rhs)
{
    return lhs.value() < Time<Unit> { rhs }.value();
}

template <typename Unit, typename OtherUnit>
constexpr bool operator> (const Time<Unit>& lhs, const Time<OtherUnit>& rhs)
{
    return rhs < lhs;
}

template <typename Unit, typename OtherUnit>
constexpr bool operator<= (const Time<Unit>& lhs, const Time<OtherUnit>& rhs)
{
    return ! (lhs > rhs);
}

template <typename Unit, typename OtherUnit>
constexpr bool operator>= (const Time<Unit>& lhs, const Time<OtherUnit>& rhs)
{
    return ! (lhs < rhs);
}
} // namespace chowdsp::Units
