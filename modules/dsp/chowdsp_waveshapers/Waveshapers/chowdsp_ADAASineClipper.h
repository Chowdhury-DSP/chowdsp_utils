#pragma once

namespace chowdsp
{
/**
 * Sinusoidal soft clipper, based on the "Soft Sine" saturator in Ableton.
 *
 * Reference: https://www.desmos.com/calculator/rdkdsmrrtr
 */
template <typename T, ADAAWaveshaperMode mode = ADAAWaveshaperMode::MinusX>
class ADAASineClipper : public ADAAWaveshaper<T, mode>
{
public:
    explicit ADAASineClipper (LookupTableCache* lutCache = nullptr, T range = (T) 10, int N = 1 << 18) : ADAAWaveshaper<T, mode> (lutCache, "chowdsp_sine_clipper")
    {
        using chowdsp::Math::sign;
        using chowdsp::Power::ipow;
        static constexpr auto halfPi = juce::MathConstants<double>::halfPi;
        static constexpr auto twoOverPi = 1.0 / halfPi;
        static constexpr auto fourOverPiSq = ipow<2> (twoOverPi);
        this->initialise (
            [] (auto x)
            {
                return std::abs (x) < 1.0
                           ? std::sin (halfPi * x)
                           : sign (x);
            },
            [] (auto x)
            {
                return std::abs (x) < 1.0
                           ? -twoOverPi * std::cos (halfPi * x)
                           : sign (x) * x - 1.0;
            },
            [] (auto x)
            {
                return std::abs (x) < 1.0
                           ? -fourOverPiSq * std::sin (halfPi * x)
                           : (sign (x) * ipow<2> (x) / 2.0) - x + sign (x) * (0.5 - fourOverPiSq);
            },
            -range,
            range,
            N);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAASineClipper)
};
} // namespace chowdsp
