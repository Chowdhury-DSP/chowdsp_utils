#include <benchmark/benchmark.h>

#include <juce_audio_basics/juce_audio_basics.h>
#include <chowdsp_math/chowdsp_math.h>

#include "bench_utils.h"

constexpr int N = 25000;
const auto dbFloat = bench_utils::makeRandomVector<float> (N);
auto outVecFloat = std::vector<float> (N, 0.0f);

static void juceDB2Gain (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (dbFloat.begin(), dbFloat.end(), outVecFloat.begin(), [] (auto x)
                        { return juce::Decibels::decibelsToGain (x); });
    }
}
BENCHMARK (juceDB2Gain)->MinTime (1);

static void juceGain2DB (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (dbFloat.begin(), dbFloat.end(), outVecFloat.begin(), [] (auto x)
                        { return juce::Decibels::gainToDecibels (x); });
    }
}
BENCHMARK (juceGain2DB)->MinTime (1);

static void chowDB2Gain (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (dbFloat.begin(), dbFloat.end(), outVecFloat.begin(), [] (auto x)
                        { return chowdsp::DecibelsApprox::decibelsToGain (x); });
    }
}
BENCHMARK (chowDB2Gain)->MinTime (1);

static void chowGain2DB (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (dbFloat.begin(), dbFloat.end(), outVecFloat.begin(), [] (auto x)
                        { return chowdsp::DecibelsApprox::gainToDecibels (x); });
    }
}
BENCHMARK (chowGain2DB)->MinTime (1);

static void squareRef (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (dbFloat.begin(), dbFloat.end(), outVecFloat.begin(), [] (auto x)
                        { return x * x; });
    }
}
BENCHMARK (squareRef)->MinTime (1);

BENCHMARK_MAIN();
