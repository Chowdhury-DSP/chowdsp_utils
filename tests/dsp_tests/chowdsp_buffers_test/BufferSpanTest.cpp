#include <CatchUtils.h>
#include <chowdsp_buffers/chowdsp_buffers.h>

TEMPLATE_TEST_CASE ("Buffer Span Test",
                    "[dsp][buffer]",
                    chowdsp::Buffer<float>,
                    (chowdsp::StaticBuffer<float, 2, 4>) )
{
    using BufferType = TestType;
    BufferType buffer { 2, 4 };

    const auto testSpan = [] (auto& testBuffer)
    {
        {
            auto writeSpan0 = testBuffer.getWriteSpan (0);
            std::fill (writeSpan0.begin(), writeSpan0.end(), 1.0f);
        }

        {
            auto writeSpan1 = testBuffer.getWriteSpan (1);
            std::fill (writeSpan1.begin(), writeSpan1.end(), 2.0f);
        }

        {
            auto readSpan0 = testBuffer.getReadSpan (0);
            REQUIRE (juce::exactlyEqual (std::accumulate (readSpan0.begin(), readSpan0.end(), 0.0f), 4.0f));
        }

        {
            auto readSpan1 = testBuffer.getReadSpan (1);
            REQUIRE (juce::exactlyEqual (std::accumulate (readSpan1.begin(), readSpan1.end(), 0.0f), 8.0f));
        }
    };

    SECTION ("Buffer Span")
    {
        testSpan (buffer);
    }

    SECTION ("Buffer View Span")
    {
        chowdsp::BufferView<float> view { buffer };
        testSpan (view);
    }
}
