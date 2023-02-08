#include <CatchUtils.h>
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
    using FloatType = float;

    void prepare (const juce::dsp::ProcessSpec&) {}
    void reset() {}

    void processBlock (const chowdsp::BufferView<float>& buffer) const
    {
        if (! onOff)
            buffer.clear();
    }

    bool onOff = true;
};

TEST_CASE ("Linear Phase EQ Test", "[dsp][EQ]")
{
    SECTION ("Process Test")
    {
        static constexpr int FIRLength = 128;
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
                REQUIRE_MESSAGE (sample == Catch::Approx { 1.0f }.margin (1.0e-3f), "Shifted impulse is incorrect!");
            else
                REQUIRE_MESSAGE (sample < 1.0e-3f, "Signal other than the impulse was detected at index " + juce::String (i) + "!");
        }

        REQUIRE_MESSAGE (testEQ.getLatencySamples() == FIRLength / 2, "Reported latency is incorrect!");
    }

    SECTION ("Parameters Test")
    {
        static constexpr int FIRLength = 16;
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
        REQUIRE_MESSAGE (processBlock() > 0.9f, "Processing ON is incorrect!");

        testEQ.setParameters ({ false });
        testEQ.setParameters ({ false });
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        REQUIRE_MESSAGE (processBlock() < 0.1f, "Processing OFF is incorrect!");
    }

    SECTION ("FIR Length Test")
    {
        static constexpr int FIRLength = 16;
        chowdsp::EQ::LinearPhaseEQ<NotAFilter, FIRLength> testEQ;
        testEQ.updatePrototypeEQParameters = [] (auto& eq, auto& params)
        { eq.onOff = params.onOff; };

        {
            testEQ.prepare ({ 48000.0, Constants::blockSize, 1 }, { true });
            REQUIRE_MESSAGE (testEQ.getLatencySamples() == FIRLength / 2, "Latency at 48 kHz is incorrect!");
        }

        {
            testEQ.prepare ({ 44100.0, Constants::blockSize, 1 }, { true });
            REQUIRE_MESSAGE (testEQ.getLatencySamples() == FIRLength / 2, "Latency at 44.1 kHz is incorrect!");
        }

        {
            testEQ.prepare ({ 96000.0, Constants::blockSize, 1 }, { true });
            REQUIRE_MESSAGE (testEQ.getLatencySamples() == FIRLength, "Latency at 96 kHz is incorrect!");
        }
    }
}
