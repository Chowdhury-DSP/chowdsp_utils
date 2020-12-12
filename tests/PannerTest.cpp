#include <JuceHeader.h>
#include "test_utils.h"

namespace
{
    constexpr float fs = 48000.0f;
    constexpr float freq = 100.0f;
    constexpr int numSamples = 512;
    constexpr float len = (float) numSamples / fs;
}

class PannerTest : public UnitTest
{
public:
    PannerTest() : UnitTest ("Panner Test") {}

    void bufferTest()
    {
        // set up panners
        chowdsp::Panner<float> panner;
        panner.setPan (-1.0f); // full left
        panner.setRule (chowdsp::Panner<float>::Rule::linear);
        panner.prepare ({ (double) fs, (uint32) numSamples, 1 });

        // set up buffers
        auto inBuffer = test_utils::makeSineWave (freq, fs, len);
        dsp::AudioBlock<float> inBlock (inBuffer);
        AudioBuffer<float> outBuffer (2, inBuffer.getNumSamples());
        dsp::AudioBlock<float> outBlock (outBuffer);

        // process
        panner.process<dsp::ProcessContextNonReplacing<float>> ({ inBlock, outBlock });
        auto leftMag = outBuffer.getMagnitude (0, 0, numSamples);
        auto rightMag = outBuffer.getMagnitude (1, 0, numSamples);

        expectEquals (leftMag, 2.0f);   // expect both channels summed into left channel
        expectEquals (rightMag, 0.0f);  // expect silence on right channel
    }

    void singleSampleTest()
    {
        // set up panners
        chowdsp::Panner<float> panner;
        panner.setPan (1.0f); // full right
        panner.setRule (chowdsp::Panner<float>::Rule::linear);
        panner.prepare ({ (double) fs, (uint32) numSamples, 1 });

        // set up buffers
        auto inBuffer = test_utils::makeSineWave (freq, fs, len);
        AudioBuffer<float> outBuffer (2, inBuffer.getNumSamples());

        // process
        auto* input = inBuffer.getReadPointer (0);
        auto* left = outBuffer.getWritePointer (0);
        auto* right = outBuffer.getWritePointer (1);
        
        for (int i = 0; i < numSamples; ++i)
            std::tie (left[i], right[i]) = panner.processSample (input[i]);
        auto leftMag = outBuffer.getMagnitude (0, 0, numSamples);
        auto rightMag = outBuffer.getMagnitude (1, 0, numSamples);

        expectEquals (leftMag, 0.0f);   // expect silence on left channel
        expectEquals (rightMag, 2.0f);  // expect both channels summed to right channel
    }

    void centerTest()
    {
        // set up panners
        chowdsp::Panner<float> panner;
        panner.setPan (0.0f); // center
        panner.setRule (chowdsp::Panner<float>::Rule::squareRoot3dB);
        panner.prepare ({ (double) fs, (uint32) numSamples, 1 });

        // set up buffers
        auto inBuffer = test_utils::makeSineWave (freq, fs, len);
        dsp::AudioBlock<float> inBlock (inBuffer);
        AudioBuffer<float> outBuffer (2, inBuffer.getNumSamples());
        dsp::AudioBlock<float> outBlock (outBuffer);

        // process
        panner.process<dsp::ProcessContextNonReplacing<float>> ({ inBlock, outBlock });
        auto leftMag = outBuffer.getMagnitude (0, 0, numSamples);
        auto rightMag = outBuffer.getMagnitude (1, 0, numSamples);

        expectWithinAbsoluteError (leftMag,  1.0f, 1.0e-3f);
        expectWithinAbsoluteError (rightMag, 1.0f, 1.0e-3f);
    }

    void runTest() override
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
