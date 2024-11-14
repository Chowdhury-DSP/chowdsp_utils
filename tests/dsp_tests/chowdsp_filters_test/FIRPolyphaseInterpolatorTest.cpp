#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

static void interpolationFilterCompare (int filterOrder, int interpolationFactor, int numChannels)
{
    const auto numSamples = 12;
    const auto halfSamples = numSamples / 2;

    chowdsp::Buffer<float> bufferIn { numChannels, numSamples };
    for (auto [ch, data] : chowdsp::buffer_iters::channels (bufferIn))
        for (auto [n, x] : chowdsp::enumerate (data))
            x = static_cast<float> (n + (size_t) ch);

    std::vector coeffs ((size_t) filterOrder, 0.0f);
    for (auto [k, h] : chowdsp::enumerate (coeffs))
        h = static_cast<float> (k);

    chowdsp::FIRFilter<float> referenceFilter { filterOrder };
    referenceFilter.prepare (numChannels);
    referenceFilter.setCoefficients (coeffs.data());

    chowdsp::Buffer<float> referenceBufferIn { numChannels, numSamples * interpolationFactor };
    chowdsp::Buffer<float> referenceBufferOut { numChannels, numSamples * interpolationFactor };
    referenceBufferIn.clear();
    for (auto [ch, data] : chowdsp::buffer_iters::channels (std::as_const (bufferIn)))
        for (auto [n, x] : chowdsp::enumerate (data))
            referenceBufferIn.getWritePointer (ch)[(int) n * interpolationFactor] = x;
    referenceFilter.processBlock (chowdsp::BufferView { referenceBufferIn, 0, halfSamples * interpolationFactor },
                                  chowdsp::BufferView { referenceBufferOut, 0, halfSamples * interpolationFactor });
    referenceFilter.processBlock (chowdsp::BufferView { referenceBufferIn, halfSamples * interpolationFactor, halfSamples * interpolationFactor },
                                  chowdsp::BufferView { referenceBufferOut, halfSamples * interpolationFactor, halfSamples * interpolationFactor });

    chowdsp::FIRPolyphaseInterpolator<float> interpolatorFilter;
    interpolatorFilter.prepare (interpolationFactor, numChannels, numSamples, coeffs);
    chowdsp::Buffer<float> testBufferOut { numChannels, numSamples * interpolationFactor };
    interpolatorFilter.processBlock (chowdsp::BufferView { bufferIn, 0, halfSamples },
                                     chowdsp::BufferView { testBufferOut, 0, halfSamples * interpolationFactor });
    interpolatorFilter.processBlock (chowdsp::BufferView { bufferIn, halfSamples, halfSamples },
                                     chowdsp::BufferView { testBufferOut, halfSamples * interpolationFactor, halfSamples * interpolationFactor });

    for (const auto [ch, refData, testData] : chowdsp::buffer_iters::zip_channels (std::as_const (referenceBufferOut),
                                                                                   std::as_const (testBufferOut)))
    {
        for (const auto [ref, test] : chowdsp::zip (refData, testData))
            REQUIRE (test == Catch::Approx { ref }.margin (1.0e-6));
    }
}

TEST_CASE ("FIR Polyphase Interpolator Test", "[dsp][filters][fir][anti-aliasing]")
{
    interpolationFilterCompare (10, 2, 1);
    interpolationFilterCompare (9, 2, 1);

    interpolationFilterCompare (16, 3, 4);
    interpolationFilterCompare (19, 3, 4);

    interpolationFilterCompare (32, 4, 2);
    interpolationFilterCompare (33, 4, 2);
}
