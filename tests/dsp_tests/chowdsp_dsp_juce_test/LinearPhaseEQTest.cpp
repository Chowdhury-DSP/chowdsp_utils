#include <TimedUnitTest.h>
#include <chowdsp_eq/chowdsp_eq.h>

namespace Constants
{
constexpr double sampleRate = 48000.0;
constexpr int blockSize = 512;
} // namespace Constants

struct NotAFilter
{
    struct Params
    {
        bool onOff;
    };

    void prepare (const juce::dsp::ProcessSpec&) {}
    void reset() {}

    void processBlock (juce::AudioBuffer<float>& buffer) const
    {
        if (! onOff)
            buffer.clear();
    }

    bool onOff = true;
};

class LinearPhaseEQTest : public TimedUnitTest
{
public:
    LinearPhaseEQTest() : TimedUnitTest ("Linear Phase EQ Test") {}

    void processTest()
    {
        constexpr int FIRLength = 128;
        chowdsp::EQ::LinearPhaseEQ<NotAFilter, FIRLength> testEQ;
        testEQ.updatePrototypeEQParameters = [] (auto& eq, auto& params)
        { eq.onOff = params.onOff; };
        testEQ.prepare ({ Constants::sampleRate, Constants::blockSize, 1 }, { true });

        juce::AudioBuffer<float> buffer (1, Constants::blockSize);
        buffer.clear();
        buffer.setSample (0, 0, 1.0f);

        auto&& block = juce::dsp::AudioBlock<float> { buffer };
        testEQ.process (juce::dsp::ProcessContextReplacing<float> { block });

        for (int i = 0; i < FIRLength; ++i)
        {
            const auto sample = buffer.getSample (0, i);

            if (i == FIRLength / 2)
                expectWithinAbsoluteError (sample, 1.0f, 1.0e-3f, "Shifted impulse is incorrect!");
            else
                expectLessThan (sample, 1.0e-3f, "Signal other than the impulse was detected at index " + juce::String (i) + "!");
        }

        expectEquals (testEQ.getLatencySamples(), FIRLength / 2, "Reported latency is incorrect!");
    }

    void parametersTest()
    {
        constexpr int FIRLength = 16;
        chowdsp::EQ::LinearPhaseEQ<NotAFilter, FIRLength> testEQ;
        testEQ.updatePrototypeEQParameters = [] (auto& eq, auto& params)
        { eq.onOff = params.onOff; };
        testEQ.prepare ({ Constants::sampleRate, Constants::blockSize, 1 }, { true });

        auto processBlock = [&testEQ]()
        {
            juce::AudioBuffer<float> buffer (1, Constants::blockSize);
            buffer.clear();
            buffer.setSample (0, 0, 1.0f);
            auto&& block = juce::dsp::AudioBlock<float> { buffer };
            testEQ.process (juce::dsp::ProcessContextReplacing<float> { block });
            return buffer.getMagnitude (0, Constants::blockSize);
        };

        testEQ.setParameters ({ true });
        expectGreaterThan (processBlock(), 0.9f, "Processing ON is incorrect!");

        testEQ.setParameters ({ false });
        testEQ.setParameters ({ false });
        juce::Thread::sleep (20);
        expectLessThan (processBlock(), 0.1f, "Processing OFF is incorrect!");
    }

    void firLengthTest()
    {
        constexpr int FIRLength = 16;
        chowdsp::EQ::LinearPhaseEQ<NotAFilter, FIRLength> testEQ;
        testEQ.updatePrototypeEQParameters = [] (auto& eq, auto& params)
        { eq.onOff = params.onOff; };

        {
            testEQ.prepare ({ 48000.0, Constants::blockSize, 1 }, { true });
            expectEquals (testEQ.getLatencySamples(), FIRLength / 2, "Latency at 48 kHz is incorrect!");
        }

        {
            testEQ.prepare ({ 44100.0, Constants::blockSize, 1 }, { true });
            expectEquals (testEQ.getLatencySamples(), FIRLength / 2, "Latency at 44.1 kHz is incorrect!");
        }

        {
            testEQ.prepare ({ 96000.0, Constants::blockSize, 1 }, { true });
            expectEquals (testEQ.getLatencySamples(), FIRLength, "Latency at 96 kHz is incorrect!");
        }
    }

    void runTestTimed() override
    {
        beginTest ("Process Test");
        processTest();

        beginTest ("Parameters Test");
        parametersTest();

        beginTest ("FIR Length Test");
        firLengthTest();
    }
};

static LinearPhaseEQTest linearPhaseEqTest;
