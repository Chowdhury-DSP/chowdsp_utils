#include <TimedUnitTest.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

class LookupTableTest : public TimedUnitTest
{
public:
    LookupTableTest() : TimedUnitTest ("Lookup Table Test")
    {
    }

    void processSingleCheckedTest (juce::Random& r)
    {
        for (int i = 0; i < TestN; ++i)
        {
            const auto testVal = (r.nextFloat() - 0.5f) * 30.0f;
            const auto actualVal = chowLUT (testVal);
            const auto expVal = juceLUT (testVal);
            expectWithinAbsoluteError (actualVal, expVal, 1.0e-18f, "Lookup Table value incorrect!");
        }
    }

    void processSingleUncheckedTest (juce::Random& r)
    {
        for (int i = 0; i < TestN; ++i)
        {
            const auto testVal = (r.nextFloat() - 0.5f) * 20.0f;
            const auto actualVal = chowLUT[testVal];
            const auto expVal = juceLUT[testVal];
            expectWithinAbsoluteError (actualVal, expVal, 1.0e-18f, "Lookup Table value incorrect!");
        }
    }

    void processBufferCheckedTest (juce::Random& r)
    {
        juce::AudioBuffer<float> inBuffer (1, TestN);
        juce::AudioBuffer<float> actualBuffer (1, TestN);
        juce::AudioBuffer<float> expBuffer (1, TestN);
        for (int i = 0; i < TestN; ++i)
            inBuffer.setSample (0, i, (r.nextFloat() - 0.5f) * 30.0f);

        chowLUT.process (inBuffer.getReadPointer (0), actualBuffer.getWritePointer (0), TestN);
        juceLUT.process (inBuffer.getReadPointer (0), expBuffer.getWritePointer (0), TestN);

        for (int i = 0; i < TestN; ++i)
        {
            const auto actualVal = actualBuffer.getSample (0, i);
            const auto expVal = expBuffer.getSample (0, i);
            expectWithinAbsoluteError (actualVal, expVal, 1.0e-18f, "Lookup Table value incorrect!");
        }
    }

    void processBufferUncheckedTest (juce::Random& r)
    {
        juce::AudioBuffer<float> inBuffer (1, TestN);
        juce::AudioBuffer<float> actualBuffer (1, TestN);
        juce::AudioBuffer<float> expBuffer (1, TestN);
        for (int i = 0; i < TestN; ++i)
            inBuffer.setSample (0, i, (r.nextFloat() - 0.5f) * 20.0f);

        chowLUT.processUnchecked (inBuffer.getReadPointer (0), actualBuffer.getWritePointer (0), TestN);
        juceLUT.processUnchecked (inBuffer.getReadPointer (0), expBuffer.getWritePointer (0), TestN);

        for (int i = 0; i < TestN; ++i)
        {
            const auto actualVal = actualBuffer.getSample (0, i);
            const auto expVal = expBuffer.getSample (0, i);
            expectWithinAbsoluteError (actualVal, expVal, 1.0e-18f, "Lookup Table value incorrect!");
        }
    }

    void runTestTimed() override
    {
        auto rand = getRandom();

        beginTest ("Process Single Checked");
        processSingleCheckedTest (rand);

        beginTest ("Process Single Unchecked");
        processSingleUncheckedTest (rand);

        beginTest ("Process Buffer Checked");
        processBufferCheckedTest (rand);

        beginTest ("Process Buffer Unchecked");
        processBufferUncheckedTest (rand);
    }

private:
    static constexpr float minVal = -10.0f;
    static constexpr float maxVal = 10.0f;
    static constexpr int N = 1 << 17;
    static constexpr int TestN = 1000;

    const juce::dsp::LookupTableTransform<float> juceLUT { [] (auto x) { return std::tanh (x); },
                                                           minVal,
                                                           maxVal,
                                                           N };
    const chowdsp::LookupTableTransform<float> chowLUT { [] (auto x) { return std::tanh (x); },
                                                         minVal,
                                                         maxVal,
                                                         N };
};

class LookupTableTest lookupTableTest;
