#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
constexpr auto _sampleRate = 1000.0;
constexpr auto _blockSize = 512;
constexpr auto testFreq = 100.0f;
} // namespace

TEST_CASE ("Sawtooth Test")
{
    SECTION ("Reference Test")
    {
        // our osc has 1/2 sample delay so, run the reference osc at 2x sample rate, and check every other.
        float phase = 0.0f;
        const auto phaseIncrement = juce::MathConstants<float>::twoPi * testFreq / float (2.0 * _sampleRate);
        auto refOsc = [&phase, phaseIncrement]() mutable
        {
            const auto y = (phase - juce::MathConstants<float>::pi) / juce::MathConstants<float>::pi;
            phase += phaseIncrement;

            while (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            return y;
        };

        chowdsp::SawtoothWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);
        REQUIRE_MESSAGE (testOsc.getFrequency() == testFreq, "Set frequency is incorrect!");

        testOsc.processSample(); // for half-sample delay?
        for (int i = 0; i < 20; ++i)
        {
            refOsc();
            REQUIRE_MESSAGE (testOsc.processSample() == Approx (refOsc()).margin (0.01f), "Generated sample is incorrect!");
        }
    }

    SECTION ("SIMD Reference Test")
    {
        // our osc has 1/2 sample delay so, run the reference osc at 2x sample rate, and check every other.
        float phase = 0.0f;
        const auto phaseIncrement = juce::MathConstants<float>::twoPi * testFreq / float (2.0 * _sampleRate);
        auto refOsc = [&phase, phaseIncrement]() mutable
        {
            const auto y = (phase - juce::MathConstants<float>::pi) / juce::MathConstants<float>::pi;
            phase += phaseIncrement;

            while (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            return y;
        };

        chowdsp::SawtoothWave<xsimd::batch<float>> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);
        REQUIRE_MESSAGE (testOsc.getFrequency().get (0) == testFreq, "Set frequency is incorrect!");

        testOsc.processSample(); // for half-sample delay
        for (int i = 0; i < 20; ++i)
        {
            refOsc();

            auto expOut = refOsc();
            auto testOut = testOsc.processSample();
            REQUIRE_MESSAGE (testOut.get (0) == Approx (expOut).margin (0.01f), "Generated sample is incorrect!");
            REQUIRE_MESSAGE (testOut.get (1) == Approx (expOut).margin (0.01f), "Generated sample is incorrect!");
        }
    }

    SECTION ("Process Replacing Test")
    {
        float phase = 0.0f;
        const auto phaseIncrement = juce::MathConstants<float>::twoPi * testFreq / float (2.0 * _sampleRate);
        auto refOsc = [&phase, phaseIncrement](float input) mutable
        {
            const auto y = (phase - juce::MathConstants<float>::pi) / juce::MathConstants<float>::pi;
            phase += phaseIncrement;

            while (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            return y + input;
        };

        chowdsp::SawtoothWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);

        chowdsp::Buffer<float> testBuffer (1, 21);
        juce::FloatVectorOperations::fill (testBuffer.getWritePointer (0), 1.0f, 21);
        testOsc.processBlock (testBuffer);

        for (int i = 0; i < 19; ++i)
        {
            refOsc (1.0f);
            auto expOut = refOsc (1.0f);
            auto actualOut = testBuffer.getReadPointer (0)[i + 1];
            REQUIRE_MESSAGE (actualOut == Approx (expOut).margin (0.01f), "Generated sample is incorrect!");
        }
    }
//
//    SECTION ("Process Non-Replacing Test")
//    {
//        juce::dsp::Oscillator<float> refOsc { [] (auto x) { return x / juce::MathConstants<float>::pi; } };
//        refOsc.prepare ({ 2.0 * _sampleRate, (juce::uint32) _blockSize, 2 });
//        refOsc.setFrequency (testFreq, true);
//
//        juce::AudioBuffer<float> inputBuffer1 (1, 40);
//        juce::FloatVectorOperations::fill (inputBuffer1.getWritePointer (0), 1.0f, 40);
//        juce::AudioBuffer<float> refBuffer (2, 40);
//        auto refBlock = juce::dsp::AudioBlock<float> { refBuffer };
//        refOsc.process (juce::dsp::ProcessContextNonReplacing<float> { inputBuffer1, refBlock });
//
//        chowdsp::SawtoothWave<float> testOsc;
//        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 2 });
//        testOsc.setFrequency (testFreq);
//
//        juce::AudioBuffer<float> inputBuffer2 (1, 21);
//        juce::FloatVectorOperations::fill (inputBuffer2.getWritePointer (0), 1.0f, 21);
//        juce::AudioBuffer<float> testBuffer (2, 21);
//        auto testBlock = juce::dsp::AudioBlock<float> { testBuffer };
//        testOsc.process (juce::dsp::ProcessContextNonReplacing<float> { inputBuffer2, testBlock });
//
//        for (int i = 0; i < 19; ++i)
//        {
//            auto expOutL = refBuffer.getSample (0, 2 * i + 1);
//            auto actualOutL = testBuffer.getSample (0, i + 1);
//            expectWithinAbsoluteError (actualOutL, expOutL, 0.01f, "Generated sample is incorrect!");
//
//            auto expOutR = refBuffer.getSample (1, 2 * i + 1);
//            auto actualOutR = testBuffer.getSample (1, i + 1);
//            expectWithinAbsoluteError (actualOutR, expOutR, 0.01f, "Generated sample is incorrect!");
//        }
//    }

    SECTION ("Zero Hz Test")
    {
        chowdsp::SawtoothWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (0.0f);

        for (int i = 0; i < 10; ++i)
            REQUIRE_MESSAGE (testOsc.processSample() == 0.0f, "Zero Hz output is non-zero!");
    }

//    SECTION ("Bypass Test")
//    {
//        juce::dsp::Oscillator<float> refOsc { [] (auto x) { return x / juce::MathConstants<float>::pi; } };
//        refOsc.prepare ({ 2.0 * _sampleRate, (juce::uint32) _blockSize, 1 });
//        refOsc.setFrequency (testFreq, true);
//
//        juce::AudioBuffer<float> refBuffer (1, 40);
//        juce::FloatVectorOperations::fill (refBuffer.getWritePointer (0), 1.0f, 40);
//        auto refBlock = juce::dsp::AudioBlock<float> { refBuffer };
//        refOsc.process (juce::dsp::ProcessContextReplacing<float> { refBlock });
//
//        chowdsp::SawtoothWave<float> testOsc;
//        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
//        testOsc.setFrequency (testFreq);
//
//        juce::AudioBuffer<float> testBuffer (1, 21);
//        juce::FloatVectorOperations::fill (testBuffer.getWritePointer (0), 1.0f, 21);
//
//        auto testBlock = juce::dsp::AudioBlock<float> { testBuffer };
//        auto&& block1 = testBlock.getSubBlock (0, 11);
//        auto&& ctx1 = juce::dsp::ProcessContextReplacing<float> (block1);
//        ctx1.isBypassed = true;
//        testOsc.process (ctx1);
//
//        auto&& block2 = testBlock.getSubBlock (11, 10);
//        auto&& ctx2 = juce::dsp::ProcessContextReplacing<float> (block2);
//        ctx2.isBypassed = false;
//        testOsc.process (ctx2);
//
//        int i = 0;
//        for (; i < 10; ++i)
//        {
//            auto actualOut = testBuffer.getSample (0, i + 1);
//            expectWithinAbsoluteError (actualOut, 1.0f, 0.01f, "Bypassed sample is incorrect!");
//        }
//
//        for (; i < 19; ++i)
//        {
//            auto expOut = refBuffer.getSample (0, 2 * i + 1);
//            auto actualOut = testBuffer.getSample (0, i + 1);
//            expectWithinAbsoluteError (actualOut, expOut, 0.01f, "Generated sample is incorrect!");
//        }
//    }
}
