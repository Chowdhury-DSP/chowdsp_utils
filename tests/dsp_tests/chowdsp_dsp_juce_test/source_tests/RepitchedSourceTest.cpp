#include <TimedUnitTest.h>
#include <chowdsp_sources/chowdsp_sources.h>

/** @TODO: figure out why this test is flaky on Linux */
#if ! JUCE_LINUX

namespace Constants
{
constexpr double fs = 48000.0f;
constexpr int blockSize = 256;

constexpr float sineFreq = 100.0f;
} // namespace Constants

class RepitchedSourceTest : public TimedUnitTest
{
public:
    RepitchedSourceTest() : TimedUnitTest ("Repitched Source Test") {}

    struct SineProcessor : chowdsp::RepitchedSource<chowdsp::ResamplingTypes::LanczosResampler<>>
    {
        chowdsp::SineWave<float> sine;

        SineProcessor() : chowdsp::RepitchedSource<chowdsp::ResamplingTypes::LanczosResampler<>> (3.0f) {}

        void prepareRepitched (const juce::dsp::ProcessSpec& spec) override
        {
            sine.prepare (spec);
        }

        void processRepitched (juce::dsp::AudioBlock<float>& block) override
        {
            sine.process (juce::dsp::ProcessContextReplacing<float> { block });
        }
    };

    void runSineTest (float repitchFactor)
    {
        using namespace Constants;

        SineProcessor sineProc;
        sineProc.prepare ({ fs, (juce::uint32) blockSize, 1 });
        sineProc.sine.setFrequency (sineFreq);
        sineProc.setRepitchFactor (repitchFactor);
        expectEquals (sineProc.getRepitchFactor(), repitchFactor, "Set repitch factor is incorrect!");

        chowdsp::TunerProcessor<float> tuner;
        tuner.prepare (fs);
        const auto numSamples = tuner.getAutocorrelationSize();

        for (int n = 0; n < 4; ++n)
        {
            auto&& buffer = juce::AudioBuffer<float> (1, numSamples);
            buffer.clear();
            for (int i = 0; i < numSamples; i += blockSize)
            {
                const auto samplesToProcess = juce::jmin (blockSize, numSamples - i);
                auto&& block = sineProc.process (samplesToProcess);

                buffer.copyFrom (0, i, block.getChannelPointer (0), samplesToProcess);
            }

            tuner.process (buffer.getReadPointer (0));
        }

        expectWithinAbsoluteError (tuner.getCurrentFrequencyHz(), sineFreq * repitchFactor, 1.0f, "Repitched frequency is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("No Repitch Test");
        runSineTest (1.0f);

        beginTest ("Pitch Up Test");
        runSineTest (2.0f);

        beginTest ("Pitch Down Test");
        runSineTest (0.5f);
    }
};

static RepitchedSourceTest repitchedSourceTest;

#endif
