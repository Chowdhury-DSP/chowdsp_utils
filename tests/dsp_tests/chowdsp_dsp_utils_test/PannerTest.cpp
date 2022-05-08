#include <test_utils.h>
#include <TimedUnitTest.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
constexpr float fs = 48000.0f;
constexpr float freq = 100.0f;
constexpr int nSamples = 512;
constexpr float lenSeconds = (float) nSamples / fs;
} // namespace

class PannerTest : public TimedUnitTest
{
public:
    PannerTest() : TimedUnitTest ("Panner Test") {}

    void bufferTest()
    {
        // set up panners
        chowdsp::Panner<float> panner;
        panner.setPan (-1.0f); // full left
        panner.setRule (chowdsp::Panner<float>::Rule::linear);
        panner.prepare ({ (double) fs, (juce::uint32) nSamples, 1 });

        // set up buffers
        auto inBuffer = test_utils::makeSineWave (freq, fs, lenSeconds);
        juce::dsp::AudioBlock<float> inBlock (inBuffer);
        juce::AudioBuffer<float> outBuffer (2, inBuffer.getNumSamples());
        juce::dsp::AudioBlock<float> outBlock (outBuffer);

        // process
        panner.process<juce::dsp::ProcessContextNonReplacing<float>> ({ inBlock, outBlock });
        auto leftMag = outBuffer.getMagnitude (0, 0, nSamples);
        auto rightMag = outBuffer.getMagnitude (1, 0, nSamples);

        expectEquals (leftMag, 2.0f); // expect both channels summed into left channel
        expectEquals (rightMag, 0.0f); // expect silenSecondsce on right channel
    }

    void singleSampleTest()
    {
        // set up panners
        chowdsp::Panner<float> panner;
        panner.setPan (1.0f); // full right
        panner.setRule (chowdsp::Panner<float>::Rule::linear);
        panner.prepare ({ (double) fs, (juce::uint32) nSamples, 1 });

        // set up buffers
        auto inBuffer = test_utils::makeSineWave (freq, fs, lenSeconds);
        juce::AudioBuffer<float> outBuffer (2, inBuffer.getNumSamples());

        // process
        auto* input = inBuffer.getReadPointer (0);
        auto* left = outBuffer.getWritePointer (0);
        auto* right = outBuffer.getWritePointer (1);

        for (int i = 0; i < nSamples; ++i)
            std::tie (left[i], right[i]) = panner.processSample (input[i]);
        auto leftMag = outBuffer.getMagnitude (0, 0, nSamples);
        auto rightMag = outBuffer.getMagnitude (1, 0, nSamples);

        expectEquals (leftMag, 0.0f); // expect silenSecondsce on left channel
        expectEquals (rightMag, 2.0f); // expect both channels summed to right channel
    }

    void centerTest()
    {
        // set up panners
        chowdsp::Panner<float> panner;
        panner.setPan (0.0f); // center
        panner.setRule (chowdsp::Panner<float>::Rule::squareRoot3dB);
        panner.prepare ({ (double) fs, (juce::uint32) nSamples, 1 });

        // set up buffers
        auto inBuffer = test_utils::makeSineWave (freq, fs, lenSeconds);
        juce::dsp::AudioBlock<float> inBlock (inBuffer);
        juce::AudioBuffer<float> outBuffer (2, inBuffer.getNumSamples());
        juce::dsp::AudioBlock<float> outBlock (outBuffer);

        // process
        panner.process<juce::dsp::ProcessContextNonReplacing<float>> ({ inBlock, outBlock });
        auto leftMag = outBuffer.getMagnitude (0, 0, nSamples);
        auto rightMag = outBuffer.getMagnitude (1, 0, nSamples);

        expectWithinAbsoluteError (leftMag, 1.0f, 1.0e-3f);
        expectWithinAbsoluteError (rightMag, 1.0f, 1.0e-3f);
    }

    void runTestTimed() override
    {
        beginTest ("Buffer Test");
        bufferTest();

        beginTest ("Single-sample Test");
        singleSampleTest();

        beginTest ("Center Test");
        centerTest();
    }
};

static PannerTest pannerTest;
