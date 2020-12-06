#include <JuceHeader.h>

namespace
{
    constexpr double fs = 44100.0;
    constexpr double freq1 = 100.0;
    constexpr double freq2 = 200.0;
    constexpr double err = 1.0e-3;
}

class SineTest : public UnitTest
{
public:
    SineTest() : UnitTest ("Sine Test") {}

    void singleSampleTest()
    {
        chowdsp::SineWave<double> chowSine;
        dsp::ProcessSpec spec { fs, 512, 1 };
        chowSine.prepare (spec);
        chowSine.setFrequency (freq1);

        for (int i = 0; i < 2000; i++)
        {
            auto actual = chowSine.processSample();
            auto expected = std::sin (MathConstants<double>::twoPi * freq1 * i / fs);
            expectWithinAbsoluteError (actual, expected, err, "Sine Wave is inaccurate");
        }
    }

    void bufferTest()
    {
        chowdsp::SineWave<double> chowSine;

        constexpr int blockSize = 512;
        constexpr int numChannels = 4;
        dsp::ProcessSpec spec { fs, blockSize, numChannels };
        chowSine.prepare (spec);
        chowSine.setFrequency (freq2);

        AudioBuffer<double> chowBuffer (numChannels, 2 * blockSize);
        AudioBuffer<double> refBuffer  (numChannels, 2 * blockSize);

        chowBuffer.clear();
        refBuffer.clear();

        // process reference buffer
        for (int ch = 0; ch < numChannels; ++ch)
            for (int i = 0; i < refBuffer.getNumSamples(); ++i)
                refBuffer.setSample (ch, i, std::sin (MathConstants<double>::twoPi * freq2 * i / fs));

        // process with chowdsp::SineWave
        for (int i = 0; i < 2; ++i)
        {
            dsp::AudioBlock<double> block (chowBuffer.getArrayOfWritePointers(), numChannels,
                                           (size_t) i * blockSize, (size_t) blockSize);
            dsp::ProcessContextReplacing<double> context (block);
            chowSine.process (context);
        }

        // check for accuracy
        for (int ch = 0; ch < numChannels; ++ch)
        {
            for (int i = 0; i < 2 * blockSize; ++i)
            {
                expectWithinAbsoluteError (chowBuffer.getSample (ch, i),
                                           refBuffer.getSample (ch, i),
                                           err, "Sine Wave is inaccurate");
            }
        }
    }

    void runTest() override
    {
        beginTest ("Single-Sample Processing Test");
        singleSampleTest();

        beginTest ("Buffer Processing Test");
        bufferTest();
    }
};

static SineTest sineTest;
