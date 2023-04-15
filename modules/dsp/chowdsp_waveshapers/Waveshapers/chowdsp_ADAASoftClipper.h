#pragma once

namespace chowdsp
{
/**
 * Polynomial soft clipper implemented with 2nd-order integrated waveshaping (ADAA).
 *
 * Note that this processor adds one sample of latency to the signal.
 *
 * Derivation: https://www.desmos.com/calculator/fvu6zrubon
 */
template <typename T, int degree = 3, ADAAWaveshaperMode mode = ADAAWaveshaperMode::MinusX>
class ADAASoftClipper : public ADAAWaveshaper<T, mode>
{
public:
    static_assert (degree % 2 == 1 && degree > 2, "Degree must be an odd integer, larger than 2!");

    explicit ADAASoftClipper (LookupTableCache* lutCache = nullptr, T range = (T) 10, int N = 1 << 17) : ADAAWaveshaper<T, mode> (lutCache, "chowdsp_soft_clipper_" + std::to_string (degree))
    {
        using Math::sign, Power::ipow;
        static constexpr auto D = (double) degree;
        static constexpr auto normFactor = (D - 1.0) / D;
        static constexpr auto invNormFactor = 1.0 / normFactor;
        static constexpr auto G1 = 1.0 / (2.0 * ipow<2> (normFactor)) - 1.0 / (ipow<2> (normFactor) * D * (D + 1.0));
        static constexpr auto G2 = 1.0 / (6.0 * ipow<3> (normFactor)) - 1.0 / (ipow<3> (normFactor) * D * (D + 1.0) * (D + 2.0));
        juce::ignoreUnused (G2);

        this->initialise (
            [] (auto x)
            {
                if (std::abs (x * normFactor) > 1.0)
                    return sign (x);
                else
                    return ((x * normFactor) - ipow<degree> (x * normFactor) / D) * invNormFactor;
            },
            [] (auto x)
            {
                if (std::abs (x * normFactor) > 1.0)
                    return x * sign (x) + G1 - invNormFactor;
                else
                    return ((normFactor * ipow<2> (x) / 2.0) - (ipow<degree> (normFactor) * ipow<degree + 1> (x) / (D * (D + 1.0)))) * invNormFactor;
            },
            [] (auto x)
            {
                if (std::abs (x * normFactor) > 1.0)
                    return ((ipow<2> (x) / 2.0) + G2 + (1.0 / (2.0 * ipow<2> (normFactor))) - G1 * invNormFactor) * sign (x) + x * (G1 - invNormFactor);
                else
                    return ((normFactor * ipow<3> (x) / 6.0) - (ipow<degree> (normFactor) * ipow<degree + 2> (x) / (D * (D + 1.0) * (D + 2.0)))) * invNormFactor;
            },
            -range,
            range,
            N);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAASoftClipper)
};
} // namespace chowdsp
