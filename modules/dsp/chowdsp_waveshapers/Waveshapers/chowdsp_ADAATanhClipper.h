#pragma once

namespace chowdsp
{
/**
 * Tanh soft clipper implemented with 2nd-order integrated waveshaping (ADAA).
 *
 * Note that this processor adds one sample of latency to the signal.
 */
template <typename T>
class ADAATanhClipper : public ADAAWaveshaper<T>
{
public:
    explicit ADAATanhClipper (LookupTableCache* lutCache = nullptr, T range = (T) 10, int N = 1 << 18) : ADAAWaveshaper<T> (lutCache, "tanh_clipper")
    {
        this->initialise (
            [] (auto x)
            { return std::tanh (x); },
            [] (auto x)
            { return std::log (std::cosh (x)); },
            [] (auto x)
            {
                using Polylogarithm::Li2, Power::ipow;
                const auto expVal = std::exp (-2.0 * x);
                return 0.5 * (Li2 (-expVal) - x * (x + 2.0 * std::log (expVal + 1.0) - 2.0 * std::log (std::cosh (x)))) + (ipow<2> (juce::MathConstants<double>::pi) / 24.0);
            },
            -range,
            range,
            N);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAATanhClipper)
};
} // namespace chowdsp
