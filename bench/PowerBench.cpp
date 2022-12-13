#include <benchmark/benchmark.h>

#include <juce_dsp/juce_dsp.h>
#include <chowdsp_simd/chowdsp_simd.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#include <chowdsp_math/chowdsp_math.h>

#include "bench_utils.h"

constexpr int N = 25000;
const auto powerFloat = bench_utils::makeRandomVector<float> (N);
auto outVecFloat = std::vector<float> (N, 0.0f);

static void stdPow7 (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (powerFloat.begin(), powerFloat.end(), outVecFloat.begin(), [] (auto x)
                        { return std::pow (x, 7.0f); });
    }
}
BENCHMARK (stdPow7)->MinTime (1);

static void stdPow16 (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (powerFloat.begin(), powerFloat.end(), outVecFloat.begin(), [] (auto x)
                        { return std::pow (x, 16.0f); });
    }
}
BENCHMARK (stdPow16)->MinTime (1);

static void scalarPow7 (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (powerFloat.begin(), powerFloat.end(), outVecFloat.begin(), [] (auto x)
                        { return chowdsp::Power::ipow<7> (x); });
    }
}
BENCHMARK (scalarPow7)->MinTime (1);

static void scalarPow16 (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (powerFloat.begin(), powerFloat.end(), outVecFloat.begin(), [] (auto x)
                        { return chowdsp::Power::ipow<16> (x); });
    }
}
BENCHMARK (scalarPow16)->MinTime (1);

static void vectorPow7 (benchmark::State& state)
{
    for (auto _ : state)
    {
        chowdsp::FloatVectorOperations::integerPower (outVecFloat.data(), powerFloat.data(), 7, N);
    }
}
BENCHMARK (vectorPow7)->MinTime (1);

static void vectorPow16 (benchmark::State& state)
{
    for (auto _ : state)
    {
        chowdsp::FloatVectorOperations::integerPower (outVecFloat.data(), powerFloat.data(), 16, N);
    }
}
BENCHMARK (vectorPow16)->MinTime (1);

BENCHMARK_MAIN();
