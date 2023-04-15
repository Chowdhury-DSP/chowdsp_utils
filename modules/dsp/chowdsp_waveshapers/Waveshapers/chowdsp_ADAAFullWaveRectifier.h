#pragma once

namespace chowdsp
{
/**
 * Full Wave Recitifier implemented with 2nd-order integrated waveshaping (ADAA).
 *
 * Note that this processor adds one sample of latency to the signal.
 *
 * Derivation: https://www.desmos.com/calculator/jxnfvgqrvs
 */
template <typename T, ADAAWaveshaperMode mode = ADAAWaveshaperMode::MinusX>
class ADAAFullWaveRectifier : public ADAAWaveshaper<T, mode>
{
public:
    explicit ADAAFullWaveRectifier (LookupTableCache* lutCache = nullptr, T range = (T) 10, int N = 1 << 17) : ADAAWaveshaper<T, mode> (lutCache, "hard_clipper")
    {
        using Math::sign;
        this->initialise (
            [] (auto x)
            { return std::abs (x); },
            [] (auto x)
            {
                return Math::sign (x) * Power::ipow<2> (x) / 2.0;
            },
            [] (auto x)
            {
                return Math::sign (x) * Power::ipow<3> (x) / 6.0;
            },
            -range,
            range,
            N);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAAFullWaveRectifier)
};
} // namespace chowdsp
