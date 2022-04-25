#include <TimedUnitTest.h>

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

        void prepareRepitched (const dsp::ProcessSpec& spec) override
        {
            sine.prepare (spec);
        }

        void processRepitched (dsp::AudioBlock<float>& block) override
        {
            sine.process (dsp::ProcessContextReplacing<float> { block });
        }
    };

    void runSineTest (float repitchFactor)
    {
        using namespace Constants;

        SineProcessor sineProc;
        sineProc.prepare ({ fs, (uint32) blockSize, 1 });
        sineProc.sine.setFrequency (sineFreq);
        sineProc.setRepitchFactor (repitchFactor);

        chowdsp::TunerProcessor<float> tuner;
        tuner.prepare (fs);
        const auto numSamples = tuner.getAutocorrelationSize();

        auto&& buffer = AudioBuffer<float> (1, numSamples);
        buffer.clear();
        for (int i = 0; i < numSamples; i += blockSize)
        {
            const auto samplesToProcess = jmin (blockSize, numSamples - i);
            auto&& block = sineProc.process (samplesToProcess);

            buffer.copyFrom (0, i, block.getChannelPointer (0), samplesToProcess);
        }

        tuner.process (buffer.getReadPointer (0));
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
