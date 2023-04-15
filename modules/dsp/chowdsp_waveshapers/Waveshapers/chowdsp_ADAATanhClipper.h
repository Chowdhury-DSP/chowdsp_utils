#pragma once

namespace chowdsp
{
/**
 * Tanh soft clipper implemented with 2nd-order integrated waveshaping (ADAA).
 *
 * Note that this processor adds one sample of latency to the signal.
 */
template <typename T, ADAAWaveshaperMode mode = ADAAWaveshaperMode::MinusX>
class ADAATanhClipper : public ADAAWaveshaper<T, mode>
{
public:
    explicit ADAATanhClipper (LookupTableCache* lutCache = nullptr, T range = (T) 10, int N = 1 << 18) : ADAAWaveshaper<T, mode> (lutCache, "chowdsp_tanh_clipper")
    {
        using namespace TanhIntegrals;
        this->initialise (
            [] (auto x)
            { return std::tanh (x); },
            [] (auto x)
            { return tanhAD1 (x); },
            [] (auto x)
            { return tanhAD2 (x); },
            -range,
            range,
            N);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAATanhClipper)
};
} // namespace chowdsp
