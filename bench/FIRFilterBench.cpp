#include <unordered_map>
#include <benchmark/benchmark.h>

#include <juce_dsp/juce_dsp.h>
#include <chowdsp_filters/chowdsp_filters.h>

#include "bench_utils.h"

constexpr int blockSize = 512;
constexpr int startOrder = 16;
constexpr int endOrder = 1024;
constexpr int orderMult = 4;

static auto makeJUCEFIR (int filterOrder)
{
    const auto coefsData = bench_utils::makeRandomVector<float> (filterOrder);

    juce::dsp::FIR::Filter<float> filter;
    auto& coefs = filter.coefficients->coefficients;
    coefs.resize (filterOrder);
    std::copy (coefsData.cbegin(), coefsData.cend(), coefs.begin());

    filter.prepare ({ 48000.0, (juce::uint32) blockSize, 1 });

    return filter;
}

static auto makeJUCEFIRChoices()
{
    std::unordered_map<int, juce::dsp::FIR::Filter<float>> choices;

    for (int order = startOrder; order <= endOrder; order *= orderMult)
        choices.emplace (std::make_pair (order, makeJUCEFIR (order)));

    return choices;
}

auto juceFIRChoices = makeJUCEFIRChoices();

static auto makeConvolution (int filterOrder)
{
    const auto coefsData = bench_utils::makeRandomVector<float> (filterOrder);

    juce::AudioBuffer<float> coeffsBuffer;
    coeffsBuffer.setSize (1, filterOrder);
    coeffsBuffer.copyFrom (0, 0, coefsData.data(), filterOrder);

    using juce::dsp::Convolution;
    auto convolution = std::make_unique<Convolution>();
    convolution->loadImpulseResponse (std::move (coeffsBuffer), 48000.0, Convolution::Stereo::no, Convolution::Trim::no, Convolution::Normalise::no);
    convolution->prepare ({ 48000.0, (juce::uint32) blockSize, 1 });

    return convolution;
}

static auto makeChowFIR (int order)
{
    const auto coefsData = bench_utils::makeRandomVector<float> (order);

    chowdsp::FIRFilter<float> filter { order };
    filter.setCoefficients (coefsData.data());

    return filter;
}

static auto makeChowFIRChoices()
{
    std::unordered_map<int, chowdsp::FIRFilter<float>> choices;

    for (int order = startOrder; order <= endOrder; order *= orderMult)
        choices.emplace (std::make_pair (order, makeChowFIR (order)));

    return choices;
}

auto chowFIRChoices = makeChowFIRChoices();

static auto makeChowPolyphaseDecimFIR (int order)
{
    const auto coefsData = bench_utils::makeRandomVector<float> (order);

    chowdsp::FIRPolyphaseDecimator<float> filter {};
    filter.prepare (2, 1, blockSize, coefsData);

    return filter;
}

static auto makeChowPolyphaseDecimFIRChoices()
{
    std::unordered_map<int, chowdsp::FIRPolyphaseDecimator<float>> choices;

    for (int order = startOrder; order <= endOrder; order *= orderMult)
        choices.emplace (std::make_pair (order, makeChowPolyphaseDecimFIR (order)));

    return choices;
}

auto chowPolyphaseDecimFIRChoices = makeChowPolyphaseDecimFIRChoices();

static auto makeChowPolyphaseInterpFIR (int order)
{
    const auto coefsData = bench_utils::makeRandomVector<float> (order);

    chowdsp::FIRPolyphaseInterpolator<float> filter {};
    filter.prepare (2, 1, blockSize / 2, coefsData);

    return filter;
}

static auto makeChowPolyphaseInterpFIRChoices()
{
    std::unordered_map<int, chowdsp::FIRPolyphaseInterpolator<float>> choices;

    for (int order = startOrder; order <= endOrder; order *= orderMult)
        choices.emplace (std::make_pair (order, makeChowPolyphaseInterpFIR (order)));

    return choices;
}

auto chowPolyphaseInterpFIRChoices = makeChowPolyphaseInterpFIRChoices();

static auto makeAudioBuffer()
{
    auto bufferData = bench_utils::makeRandomVector<float> (blockSize);

    juce::AudioBuffer<float> buffer;
    buffer.setSize (1, blockSize);
    buffer.copyFrom (0, 0, bufferData.data(), blockSize);

    return buffer;
}
auto audioBuffer = makeAudioBuffer();

static void JuceFIR (benchmark::State& state)
{
    auto& fir = juceFIRChoices[(int) state.range (0)];
    for (auto _ : state)
    {
        auto&& audioBlock = juce::dsp::AudioBlock<float> { audioBuffer };
        fir.process (juce::dsp::ProcessContextReplacing<float> { audioBlock });
    }
}
BENCHMARK (JuceFIR)->MinTime (1)->RangeMultiplier (orderMult)->Range (startOrder, endOrder);

static void ConvFIR (benchmark::State& state)
{
    auto fir = makeConvolution ((int) state.range (0));
    for (auto _ : state)
    {
        auto&& audioBlock = juce::dsp::AudioBlock<float> { audioBuffer };
        fir->process (juce::dsp::ProcessContextReplacing<float> { audioBlock });
    }
}
BENCHMARK (ConvFIR)->MinTime (1)->RangeMultiplier (orderMult)->Range (startOrder, endOrder);

static void ChowFIR (benchmark::State& state)
{
    auto& fir = chowFIRChoices[(int) state.range (0)];
    for (auto _ : state)
    {
        fir.processBlock (audioBuffer);
    }
}
BENCHMARK (ChowFIR)->MinTime (1)->RangeMultiplier (orderMult)->Range (startOrder, endOrder);

static void ChowPolyphaseDecimFIR (benchmark::State& state)
{
    auto& fir = chowPolyphaseDecimFIRChoices[(int) state.range (0)];
    for (auto _ : state)
    {
        fir.processBlock (audioBuffer, audioBuffer);
    }
}
BENCHMARK (ChowPolyphaseDecimFIR)->MinTime (1)->RangeMultiplier (orderMult)->Range (startOrder, endOrder);

static void ChowPolyphaseInterpFIR (benchmark::State& state)
{
    auto& fir = chowPolyphaseInterpFIRChoices[(int) state.range (0)];
    for (auto _ : state)
    {
        fir.processBlock (chowdsp::BufferView { audioBuffer, 0, blockSize / 2 }, audioBuffer);
    }
}
BENCHMARK (ChowPolyphaseInterpFIR)->MinTime (1)->RangeMultiplier (orderMult)->Range (startOrder, endOrder);

BENCHMARK_MAIN();
