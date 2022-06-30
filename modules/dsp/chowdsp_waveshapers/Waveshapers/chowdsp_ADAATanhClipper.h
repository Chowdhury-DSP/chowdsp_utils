#pragma once

namespace chowdsp
{
template <typename T>
class ADAATanhClipper : public ADAAWaveshaper<T>
{
public:
    explicit ADAATanhClipper (T range = (T) 10, int N = 1 << 18)
    {
        this->initialise (
            [] (auto x)
            { return std::tanh (x); },
            [] (auto x)
            { return std::log (std::cosh (x)); },
            [] (auto x)
            {
                bool inRange = std::abs (x) <= 1.0;
                return inRange ? x * x * x / 6.0 : ((x * x / 2.0) + (1.0 / 6.0)) * sign (x) - (x / 2.0);
            },
            -range,
            range,
            N);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAATanhClipper)
};
} // namespace chowdsp
