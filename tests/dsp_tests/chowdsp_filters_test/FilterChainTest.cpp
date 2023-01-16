#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 500.0f;
constexpr float Qval = 0.7071f;
} // namespace Constants

static void checkBufferLevel (const chowdsp::Buffer<float>& buffer, float expGainDB, float maxError, const std::string& message)
{
    const auto halfSamples = buffer.getNumSamples() / 2;
    auto magDB = juce::Decibels::gainToDecibels (chowdsp::BufferMath::getMagnitude (buffer, halfSamples, halfSamples));
    REQUIRE_MESSAGE (magDB == Catch::Approx (expGainDB).margin (maxError), message);
}

TEST_CASE ("Filter Chain Test", "[dsp][filters]")
{
    SECTION ("Second Order Filter Test")
    {
        using namespace Constants;
        using FilterType = chowdsp::SecondOrderLPF<float>;
        chowdsp::FilterChain<FilterType, FilterType> filters;
        filters.reset();
        filters.get<0>().calcCoefs (fc, Qval, fs);
        filters.get<1>().calcCoefs (fc, Qval, fs);

        {
            auto buffer = test_utils::makeSineWave<float> (fc, fs, 1.0f);
            auto* bufferPtr = buffer.getWritePointer (0);
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                bufferPtr[n] = filters.processSample (bufferPtr[n]);

            checkBufferLevel (buffer, -6.0f, 0.05f, "SecondOrderLPF processSample() is incorrect!");
        }

        {
            auto buffer = test_utils::makeSineWave<float> (fc, fs, 1.0f);
            filters.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());
            checkBufferLevel (buffer, -6.0f, 0.05f, "SecondOrderLPF processBlock() is incorrect!");
        }
    }

    SECTION ("Nth Order Filter Test")
    {
        using namespace Constants;
        using FilterType = chowdsp::NthOrderFilter<float, 4>;
        chowdsp::FilterChain<FilterType, FilterType> filters;

        filters.doForEachFilter ([] (auto& filt)
                                 {
                                         filt.prepare ({ fs, 100000, 1 });
                                         filt.setCutoffFrequency (fc); });

        auto buffer = test_utils::makeSineWave<float> (2 * fc, fs, 1.0f);
        filters.processBlock<float> (buffer);

        checkBufferLevel (buffer, -48.0f, 0.5f, "NthOrderFilter process() is incorrect!");
    }
}
