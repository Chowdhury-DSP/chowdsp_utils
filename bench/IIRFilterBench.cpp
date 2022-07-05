#include <unordered_map>
#include <benchmark/benchmark.h>

#include <juce_dsp/juce_dsp.h>
#include <chowdsp_filters/chowdsp_filters.h>

#include "bench_utils.h"

constexpr int blockSize = 512;
constexpr int numSecondOrderSections = 10;

static auto makeAudioBuffer()
{
    auto bufferData = bench_utils::makeRandomVector<float> (blockSize);

    juce::AudioBuffer<float> buffer;
    buffer.setSize (1, blockSize);
    buffer.copyFrom (0, 0, bufferData.data(), blockSize);

    return buffer;
}
auto audioBuffer = makeAudioBuffer();

static void JuceIIR (benchmark::State& state)
{
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (48000.0, 100.0f, 1.0f);
    filter.prepare ({ 48000.0, (juce::uint32) blockSize, 1 });
    for (auto _ : state)
    {
        auto&& audioBlock = juce::dsp::AudioBlock<float> { audioBuffer };

        for (int i = 0; i < numSecondOrderSections; ++i)
            filter.process (juce::dsp::ProcessContextReplacing<float> { audioBlock });
    }
}
BENCHMARK (JuceIIR)->MinTime (5);

static void ChowIIR (benchmark::State& state)
{
    chowdsp::SecondOrderLPF<float> filter;
    filter.calcCoefs (100.0f, 1.0f, 48000.0f);
    filter.prepare (1);
    for (auto _ : state)
    {
        auto&& audioBlock = juce::dsp::AudioBlock<float> { audioBuffer };

        for (int i = 0; i < numSecondOrderSections; ++i)
            filter.processBlock (audioBlock);
    }
}
BENCHMARK (ChowIIR)->MinTime (5);

BENCHMARK_MAIN();
