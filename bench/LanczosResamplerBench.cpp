#include <benchmark/benchmark.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include "bench_utils.h"

// Compare the same filter and channel data, changing only coefficient sharing.
template <bool shared>
static void LanczosChannels (benchmark::State& state)
{
    using Resampler = chowdsp::ResamplingTypes::LanczosResampler<2048, 8>;
    const auto numChannels = (size_t) state.range (0);
    const auto blockSize = (size_t) state.range (1);
    std::array<Resampler, 5> resamplers;
    std::array<std::vector<float>, 5> inputs, outputs;
    std::array<Resampler::Channel, 5> channels;
    for (size_t c = 0; c < numChannels; ++c)
    {
        resamplers[c].prepare (48000.0, 6.0);
        inputs[c] = bench_utils::makeRandomVector<float> ((int) blockSize);
        outputs[c].resize (blockSize * 6 + 8);
        channels[c] = { &resamplers[c], inputs[c].data(), outputs[c].data() };
    }
    for (auto _ : state)
    {
        if constexpr (shared)
        {
            benchmark::DoNotOptimize (Resampler::processChannels (channels.data(), numChannels, blockSize));
        }
        else
        {
            for (size_t c = 0; c < numChannels; ++c)
            {
                benchmark::DoNotOptimize (resamplers[c].process (inputs[c].data(), outputs[c].data(), blockSize));
            }
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK_TEMPLATE (LanczosChannels, false)->ArgsProduct ({ { 1, 2, 3, 5 }, { 800, 3200 } });
BENCHMARK_TEMPLATE (LanczosChannels, true)->ArgsProduct ({ { 1, 2, 3, 5 }, { 800, 3200 } });
BENCHMARK_MAIN();
