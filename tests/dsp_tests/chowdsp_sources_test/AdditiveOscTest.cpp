#include "CatchUtils.h"
#include <chowdsp_sources/chowdsp_sources.h>
#include <iostream>

template <chowdsp::AdditiveOscSineApprox approx>
constexpr float marginForSineApprox()
{
    if constexpr (approx == chowdsp::AdditiveOscSineApprox::FullPrecision)
        return std::numeric_limits<float>::min();

    if constexpr (approx == chowdsp::AdditiveOscSineApprox::AbsApprox)
        return 0.75e-1f;

    if constexpr (approx == chowdsp::AdditiveOscSineApprox::BhaskaraApprox)
        return 5.0e-3f;
}

template <chowdsp::AdditiveOscSineApprox approx>
void testAdditiveSine()
{
    static constexpr auto fs = 1000.0f;
    static constexpr auto freq = 100.0f;
    static constexpr auto margin = marginForSineApprox<approx>();

    chowdsp::AdditiveOscillator<1, approx> osc;
    osc.setHarmonicAmplitudes ({ 1.0f });
    osc.prepare ((double) fs);
    osc.setFrequency (freq);

    const auto phaseOffest = 0.1f;
    osc.reset (phaseOffest);

    for (int i = 0; i < 10; ++i)
    {
        const auto refSample = std::sin (juce::MathConstants<float>::twoPi * freq / fs * (float) i + phaseOffest);
        const auto actualSample = osc.processSample();
        REQUIRE (actualSample == Catch::Approx (refSample).margin (margin));
    }
}

template <chowdsp::AdditiveOscSineApprox approx>
void testAdditiveSaw()
{
    static constexpr auto fs = 1000.0f;
    static constexpr auto freq = 50.0f;
    static constexpr auto margin = marginForSineApprox<approx>();
    static constexpr size_t nSines = 256;

    chowdsp::AdditiveOscillator<nSines, approx> osc;
    float additiveHarmonics[nSines] {};
    for (auto [i, amp] : chowdsp::enumerate (additiveHarmonics))
        amp = 1.0f / float (i + 1);
    osc.setHarmonicAmplitudes (additiveHarmonics);
    osc.prepare ((double) fs);
    osc.setFrequency (freq);
    osc.setFrequency (freq);

    chowdsp::StaticBuffer<float, 2, 10> testBuffer;
    testBuffer.setMaxSize (2, 10);
    osc.processBlock (testBuffer);

    for (auto [_, channelData] : chowdsp::buffer_iters::channels (testBuffer))
    {
        for (auto [i, testSample] : chowdsp::enumerate (channelData))
        {
            const auto refSample = [phi = juce::MathConstants<float>::twoPi * freq / fs * (float) i]
            {
                float y = 0.0f;
                for (size_t k = 0; k < nSines; ++k)
                {
                    const auto kp1 = float (k + 1);
                    if (freq * kp1 > 0.5f * fs)
                        break;
                    y += (1.0f / kp1) * std::sin (kp1 * phi);
                }
                return y;
            }();

            REQUIRE (testSample == Catch::Approx (refSample).margin (1.5f * margin));
        }
    }
}

TEST_CASE ("Additive Oscillator Test", "[dsp][sources]")
{
    using Approx = chowdsp::AdditiveOscSineApprox;

    SECTION ("Sine Osc")
    {
        testAdditiveSine<Approx::AbsApprox>();
        testAdditiveSine<Approx::BhaskaraApprox>();
        testAdditiveSine<Approx::FullPrecision>();
    }

    SECTION ("Saw Osc")
    {
        testAdditiveSaw<Approx::AbsApprox>();
        testAdditiveSaw<Approx::BhaskaraApprox>();
        testAdditiveSaw<Approx::FullPrecision>();
    }
}
