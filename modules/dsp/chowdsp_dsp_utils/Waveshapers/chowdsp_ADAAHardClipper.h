#pragma once

namespace chowdsp
{
template <typename T>
class ADAAHardClipper : public ADAAWaveshaper<T>
{
public:
    explicit ADAAHardClipper (T range = (T) 10, int N = 1 << 17)
    {
        using Math::sign;
        this->initialise (
            [] (T x)
            { return juce::jlimit ((T) -1, (T) 1, x); },
            [] (T x)
            {
                bool inRange = std::abs (x) <= (T) 1;
                return inRange ? x * x / (T) 2.0 : x * sign (x) - (T) 0.5;
            },
            [] (T x)
            {
                bool inRange = std::abs (x) <= (T) 1;
                return inRange ? x * x * x / (T) 6.0 : ((x * x / (T) 2.0) + T (1.0 / 6.0)) * sign (x) - (x / (T) 2.0);
            },
            -range,
            range,
            N);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAAHardClipper)
};
} // namespace chowdsp
