#include <CatchUtils.h>
#include <chowdsp_compressor/chowdsp_compressor.h>

namespace chow_comp = chowdsp::compressor;
TEST_CASE ("Mono Compressor Test", "[dsp][compressor]")
{
    chow_comp::MonoCompressor<float,
                              chow_comp::CompressorLevelDetector<float>,
                              chow_comp::GainComputer<float>>
        compressor;
    compressor.prepare ({ 48000.0, 8192, 2 });
    compressor.params.attackMs = 10.0f;
    compressor.params.releaseMs = 100.0f;
    compressor.params.thresholdDB = -6.0f;
    compressor.params.ratio = 2.0f;
    compressor.params.kneeDB = 0.0f;
    compressor.params.autoMakeup = false;

    chowdsp::StaticBuffer<float, 2, 8192> buffer { 2, 8192 };
    for (auto [_, data] : chowdsp::buffer_iters::channels (buffer))
        std::fill (data.begin(), data.end(), 1.0f);
    compressor.processBlock (buffer, buffer);

    for (auto [_, data] : chowdsp::buffer_iters::channels (buffer))
        REQUIRE (data.back() == Catch::Approx { 0.7071f }.margin (0.001f));
}
