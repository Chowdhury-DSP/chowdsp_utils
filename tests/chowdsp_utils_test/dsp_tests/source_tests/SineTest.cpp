#include <JuceHeader.h>

namespace
{
constexpr double fs = 44100.0;
constexpr double freq1 = 100.0;
constexpr double freq2 = 200.0;
constexpr double err = 1.0e-3;
} // namespace

/** Unit tests for chowdsp::SineWave. Testing accuracy compared to std::sin for:
 *  - single samples
 *  - buffers
*/
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

        // process samples one at a time, and compare to std::sin
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
        AudioBuffer<double> refBuffer (numChannels, 2 * blockSize);

        chowBuffer.clear();
        refBuffer.clear();

        // process reference buffer
        for (int ch = 0; ch < numChannels; ++ch)
            for (int i = 0; i < refBuffer.getNumSamples(); ++i)
                refBuffer.setSample (ch, i, std::sin (MathConstants<double>::twoPi * freq2 * i / fs));

        // process with chowdsp::SineWave
        for (int i = 0; i < 2; ++i)
        {
            dsp::AudioBlock<double> block (chowBuffer.getArrayOfWritePointers(), numChannels, (size_t) i * blockSize, (size_t) blockSize);
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
                                           err,
                                           "Sine Wave is inaccurate");
            }
        }
    }

    void channelLayoutsTest()
    {
        constexpr float errF = 1.0e-3f;
        chowdsp::SineWave<float> chowSine;

        constexpr int blockSize = (int) fs;
        constexpr int numChannels1 = 1;
        constexpr int numChannels2 = 2;
        dsp::ProcessSpec spec { fs, blockSize, numChannels2 };
        chowSine.prepare (spec);
        chowSine.setFrequency (1.0f);

        AudioBuffer<float> buffer1 (numChannels1, blockSize);
        AudioBuffer<float> buffer2 (numChannels2, blockSize);

        buffer1.clear();
        buffer2.clear();

        // initial run
        {
            dsp::AudioBlock<float> block (buffer1);
            dsp::ProcessContextReplacing<float> context (block);
            chowSine.process (context);
            expectWithinAbsoluteError (buffer1.getMagnitude (0, blockSize), 1.0f, errF, "Filling original buffer incorrect!");
        }

        // 2 channels -> 4 channels
        {
            dsp::AudioBlock<float> block1 (buffer1);
            dsp::AudioBlock<float> block2 (buffer2);
            dsp::ProcessContextNonReplacing<float> context (block1, block2);
            chowSine.process (context);

            expectWithinAbsoluteError (buffer1.getMagnitude (0, blockSize), 1.0f, errF, "Out-of-place processing incorrect (original buffer)!");
            expectWithinAbsoluteError (buffer2.getMagnitude (0, 0, blockSize), 2.0f, errF, "Adding to original data incorrect!");
            expectWithinAbsoluteError (buffer2.getMagnitude (1, 0, blockSize), 1.0f, errF, "Filling new data incorrect!");
        }
    }

    void runTest() override
    {
        beginTest ("Single-Sample Processing Test");
        singleSampleTest();

        beginTest ("Buffer Processing Test");
        bufferTest();

        beginTest ("Channel Layouts Test");
        channelLayoutsTest();
    }
};

static SineTest sineTest;
