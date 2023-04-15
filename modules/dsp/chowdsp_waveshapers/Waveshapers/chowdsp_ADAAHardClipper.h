#pragma once

namespace chowdsp
{
/**
 * Hard clipper implemented with 2nd-order integrated waveshaping (ADAA).
 *
 * Note that this processor adds one sample of latency to the signal.
 */
template <typename T, ADAAWaveshaperMode mode = ADAAWaveshaperMode::MinusX>
class ADAAHardClipper : public ADAAWaveshaper<T, mode>
{
public:
    explicit ADAAHardClipper (LookupTableCache* lutCache = nullptr, T range = (T) 10, int N = 1 << 17) : ADAAWaveshaper<T, mode> (lutCache, "chowdsp_hard_clipper")
    {
        using Math::sign;
        this->initialise (
            [] (auto x)
            { return juce::jlimit (-1.0, 1.0, x); },
            [] (auto x)
            {
                bool inRange = std::abs (x) <= 1.0;
                return inRange ? x * x / 2.0 : x * sign (x) - 0.5;
            },
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAAHardClipper)
};
} // namespace chowdsp
