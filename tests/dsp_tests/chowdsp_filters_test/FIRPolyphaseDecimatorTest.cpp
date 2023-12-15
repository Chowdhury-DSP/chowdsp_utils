#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

static void decimationFilterCompare (int filterOrder, int decimationFactor, int numChannels)
{
    const auto numSamples = decimationFactor * 12;
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

    chowdsp::Buffer<float> referenceBufferOut { numChannels, numSamples };
    referenceFilter.processBlock (chowdsp::BufferView { bufferIn, 0, halfSamples },
                                  chowdsp::BufferView { referenceBufferOut, 0, halfSamples });
    referenceFilter.processBlock (chowdsp::BufferView { bufferIn, halfSamples, halfSamples },
                                  chowdsp::BufferView { referenceBufferOut, halfSamples, halfSamples });

    chowdsp::FIRPolyphaseDecimator<float> decimatorFilter;
    decimatorFilter.prepare (decimationFactor, numChannels, numSamples, coeffs);
    chowdsp::Buffer<float> testBufferOut { numChannels, numSamples / decimationFactor };
    decimatorFilter.processBlock (chowdsp::BufferView { bufferIn, 0, halfSamples },
                                  chowdsp::BufferView { testBufferOut, 0, halfSamples / decimationFactor });
    decimatorFilter.processBlock (chowdsp::BufferView { bufferIn, halfSamples, halfSamples },
                                  chowdsp::BufferView { testBufferOut, halfSamples / decimationFactor, halfSamples / decimationFactor });

    for (const auto [ch, refData, testData] : chowdsp::buffer_iters::zip_channels (std::as_const (referenceBufferOut),
                                                                                   std::as_const (testBufferOut)))
    {
        for (const auto [n, test] : chowdsp::enumerate (testData))
        {
            const auto ref = refData[n * (size_t) decimationFactor];
            REQUIRE (test == Catch::Approx { ref }.margin (1.0e-6));
        }
    }
}

TEST_CASE ("FIR Polyphase Decimator Test", "[dsp][filters][fir][anti-aliasing]")
{
    decimationFilterCompare (10, 2, 1);
    decimationFilterCompare (9, 2, 1);

    decimationFilterCompare (16, 3, 4);
    decimationFilterCompare (19, 3, 4);

    decimationFilterCompare (32, 4, 2);
    decimationFilterCompare (33, 4, 2);
}
