#pragma once

namespace chowdsp
{
enum class WernerFilterType
{
    Lowpass2,
    Bandpass2,
    Highpass2,
    Lowpass4,
};

/**
 * State Variable Filter based on the generalized SVF structure proposed by
 * Kurt Werner and Russel McClellan in this 2020 DAFx paper: https://dafx2020.mdw.ac.at/proceedings/papers/DAFx2020_paper_70.pdf).
 *
 * Rather than using the derivation provided in the reference paper, this implementation
 * uses a different derivation performed using Andy Simper's trapezoidal method. (https://www.wolframcloud.com/obj/chowdsp/Published/WernerFilter.nb)
 */
class WernerFilter
{
public:
    WernerFilter() = default;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        state.resize (spec.numChannels, {});
        fs = (float) spec.sampleRate;
    }

    void reset()
    {
        std::fill (state.begin(), state.end(), xsimd::batch<float> {});
    }

    void calcCoeffs (float fc, float r, float kHat)
    {
        const auto g = std::tan (juce::MathConstants<float>::pi * fc / fs);
        const auto gSq = g * g;
        const auto gCb = g * gSq;
        const auto rSq = r * r;
        two_r = 2.0f * r;
        kh = kHat;
        const auto two_gr = g * two_r;
        const auto four_gr = 2 * g * two_r;
        const auto gSq_oneplus_fourkhrSq = gSq * (1 + 4 * kh * rSq);
        const auto fourgSqr_plus_fourgrSq = four_gr * (g + r);
        const auto gsum1 = -g * (g + two_r + fourgSqr_plus_fourgrSq + g * gSq_oneplus_fourkhrSq);

        const auto D1 = 1 / (1 + g * (4 * r + g * (2 + gSq + four_gr + 4 * (1 + gSq * kh) * rSq)));
        const auto D2 = 1 / (1 + four_gr + 4 * gCb * r + gSq * (2 + 4 * rSq) + gSq * gSq_oneplus_fourkhrSq);

        float vinCoefsArr alignas (xsimd::default_arch::alignment())[xsimd::batch<float>::size] {};
        float f1CoefsArr alignas (xsimd::default_arch::alignment())[xsimd::batch<float>::size] {};
        float f2CoefsArr alignas (xsimd::default_arch::alignment())[xsimd::batch<float>::size] {};
        float s1CoefsArr alignas (xsimd::default_arch::alignment())[xsimd::batch<float>::size] {};
        float s2CoefsArr alignas (xsimd::default_arch::alignment())[xsimd::batch<float>::size] {};

        vinCoefsArr[TF1] = g * (1 + gSq + two_gr) * D1;
        f1CoefsArr[TF1] = gsum1 * D1;
        f2CoefsArr[TF1] = -g * (1 + two_gr + gSq_oneplus_fourkhrSq) * D1;
        s1CoefsArr[TF1] = -four_gr * g * kh * D1;
        s2CoefsArr[TF1] = -four_gr * kh * r * (1 + two_gr) * D2;

        vinCoefsArr[TF2] = g * vinCoefsArr[TF1];
        f1CoefsArr[TF2] = vinCoefsArr[TF1];
        f2CoefsArr[TF2] = g * f2CoefsArr[TF1];
        s1CoefsArr[TF2] = g * s1CoefsArr[TF1];
        s2CoefsArr[TF2] = g * s2CoefsArr[TF1];

        vinCoefsArr[TS1] = gCb * D1;
        f1CoefsArr[TS1] = gSq * D1;
        f2CoefsArr[TS1] = g * (1 + two_gr) * D1;
        s1CoefsArr[TS1] = gsum1 * D2;
        s2CoefsArr[TS1] = f2CoefsArr[TF1];

        vinCoefsArr[TS2] = g * vinCoefsArr[TS1];
        f1CoefsArr[TS2] = g * f1CoefsArr[TS1];
        f2CoefsArr[TS2] = g * f2CoefsArr[TS1];
        s1CoefsArr[TS2] = vinCoefsArr[TF1];
        s2CoefsArr[TS2] = f2CoefsArr[TF2];

        vinCoefs = xsimd::load_aligned (vinCoefsArr);
        f1Coefs = xsimd::load_aligned (f1CoefsArr);
        f2Coefs = xsimd::load_aligned (f2CoefsArr);
        s1Coefs = xsimd::load_aligned (s1CoefsArr);
        s2Coefs = xsimd::load_aligned (s2CoefsArr);
    }

    template <WernerFilterType type = WernerFilterType::Lowpass4>
    inline float processSample (int channel, float vin) noexcept
    {
        return processSampleInternal<type> (vin, state[(size_t) channel]);
    }

private:
    template <WernerFilterType type = WernerFilterType::Lowpass4>
    inline float processSampleInternal (float vin, xsimd::batch<float>& s) noexcept
    {
        float sArr alignas (xsimd::default_arch::alignment())[xsimd::batch<float>::size] {};
        s.store_aligned (sArr);

        const auto tVals = vinCoefs * vin + f1Coefs * sArr[TF1] + f2Coefs * sArr[TF2] + s1Coefs * sArr[TS1] + s2Coefs * sArr[TS2];

        float tValsArr alignas (xsimd::default_arch::alignment())[xsimd::batch<float>::size] {};
        tVals.store_aligned (tValsArr);

        float y = 0.0f;
        if constexpr (type == WernerFilterType::Lowpass2)
        {
            y = sArr[TF2] + tValsArr[TF2]; // 2nd-order lowpass output (vf2)
        }
        else if constexpr (type == WernerFilterType::Bandpass2)
        {
            y = sArr[TF1] + tValsArr[TF1]; // 2nd-order bandpass output (vf1)
        }
        else if constexpr (type == WernerFilterType::Highpass2)
        {
            const auto vg0 = -kh * (two_r * two_r) * (sArr[TS2] + tValsArr[TS2]) + vin;
            y = -sArr[TF2] - two_r * (sArr[TF1] + tValsArr[TF1]) - tValsArr[TF2] + vg0; // 2nd-order highpass output (vf0)
        }
        else if constexpr (type == WernerFilterType::Lowpass4)
        {
            y = sArr[TS2] + tValsArr[TS2]; // 4th-order lowpass output (vs2)
        }

        s = xsimd::tanh (s + 2.0f * tVals);

        return y;
    }

    // coefs
    enum
    {
        TF1,
        TF2,
        TS1,
        TS2,
    };

    xsimd::batch<float> vinCoefs {};
    xsimd::batch<float> f1Coefs {};
    xsimd::batch<float> f2Coefs {};
    xsimd::batch<float> s1Coefs {};
    xsimd::batch<float> s2Coefs {};
    float two_r = 0.0f, kh = 0.0f;

    // state
    std::vector<xsimd::batch<float>> state;

    float fs = 48000.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WernerFilter)
};
} // namespace chowdsp
