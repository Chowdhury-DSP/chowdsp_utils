#include <benchmark/benchmark.h>

#include <juce_dsp/juce_dsp.h>
#include <chowdsp_filters/chowdsp_filters.h>

#include "bench_utils.h"

constexpr int numChannels = 2;
constexpr int blockSize = 512;
constexpr size_t numFilters = 100;

auto juceInBuffer = []()
{
    juce::AudioBuffer<float> buffer { numChannels, blockSize };
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const auto randomStuff = bench_utils::makeRandomVector<float> (blockSize);
        buffer.copyFrom (ch, 0, randomStuff.data(), blockSize);
    }
    return buffer;
}();

struct OutOfPlaceFilterJUCEBuffer
{
    chowdsp::SecondOrderLPF<float> filter;
    juce::AudioBuffer<float> outBuffer;

    OutOfPlaceFilterJUCEBuffer()
    {
        filter.prepare (numChannels);
        filter.calcCoefs (1000.0f, 0.7071f, 48000.0f);

        outBuffer.setSize (numChannels, blockSize);
        outBuffer.clear();
    }

    void process (const chowdsp::BufferView<float> inBuffer) noexcept
    {
        const auto nChannels = inBuffer.getNumChannels();
        const auto numSamples = inBuffer.getNumSamples();

        for (int ch = 0; ch < nChannels; ++ch)
        {
            const auto* x = inBuffer.getReadPointer (ch);
            auto* y = inBuffer.getWritePointer (ch);

            for (int n = 0; n < numSamples; ++n)
                y[n] = filter.processSample (x[n], ch);
        }
    }
};

std::array<OutOfPlaceFilterJUCEBuffer, numFilters> juceBufferFilters {};

static void juceBufferBench (benchmark::State& state)
{
    for (auto _ : state)
    {
        juceBufferFilters[0].process (juceInBuffer);
        for (size_t n = 1; n < numFilters; ++n)
            juceBufferFilters[n].process (juceBufferFilters[n - 1].outBuffer);
    }
}
BENCHMARK (juceBufferBench)->MinTime (5);

auto chowInBuffer = []()
{
    chowdsp::Buffer<float> buffer { numChannels, blockSize };
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const auto randomStuff = bench_utils::makeRandomVector<float> (blockSize);
        juce::FloatVectorOperations::copy (buffer.getWritePointer (ch), randomStuff.data(), blockSize);
    }
    return buffer;
}();

struct OutOfPlaceFilterChowBuffer
{
    chowdsp::SecondOrderLPF<float> filter;
    chowdsp::Buffer<float> outBuffer;

    OutOfPlaceFilterChowBuffer()
    {
        filter.prepare (numChannels);
        filter.calcCoefs (1000.0f, 0.7071f, 48000.0f);

        outBuffer.setMaxSize (numChannels, blockSize);
        outBuffer.clear();
    }

    void process (const chowdsp::BufferView<float> inBuffer) noexcept
    {
        const auto nChannels = inBuffer.getNumChannels();
        const auto numSamples = inBuffer.getNumSamples();

        for (int ch = 0; ch < nChannels; ++ch)
        {
            const auto* x = inBuffer.getReadPointer (ch);
            auto* y = inBuffer.getWritePointer (ch);

            for (int n = 0; n < numSamples; ++n)
                y[n] = filter.processSample (x[n], ch);
        }
    }
};

std::array<OutOfPlaceFilterChowBuffer, numFilters> chowBufferFilters {};

static void chowBufferBench (benchmark::State& state)
{
    for (auto _ : state)
    {
        juceBufferFilters[0].process (chowInBuffer);
        for (size_t n = 1; n < numFilters; ++n)
            juceBufferFilters[n].process (juceBufferFilters[n - 1].outBuffer);
    }
}
BENCHMARK (chowBufferBench)->MinTime (5);

BENCHMARK_MAIN();
