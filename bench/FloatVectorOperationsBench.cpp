#include <algorithm>
#include <numeric>

#include <benchmark/benchmark.h>

#include <JuceHeader.h>
#include "bench_utils.h"

/** Accumulate benchmarks............................ */
constexpr int accumulateN = 10000;
const auto accumulateVecFloat = bench_utils::makeRandomVector<float> (accumulateN);

static void AccumulateSTLFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        auto sum = std::accumulate (accumulateVecFloat.begin(), accumulateVecFloat.end(), 0.0f);
        benchmark::DoNotOptimize (sum);
    }
}
BENCHMARK (AccumulateSTLFloat);

static void AccumulateChowFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        auto sum = chowdsp::FloatVectorOperations::accumulate (accumulateVecFloat.data(), accumulateN);
        benchmark::DoNotOptimize (sum);
    }
}
BENCHMARK (AccumulateChowFloat);

/** Inner Product benchmarks............................ */
constexpr int innerProdN = 50000;
const auto innerProdVecFloat1 = bench_utils::makeRandomVector<float> (innerProdN);
const auto innerProdVecFloat2 = bench_utils::makeRandomVector<float> (innerProdN);

static void InnerProdSTLFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        auto prod = std::inner_product (innerProdVecFloat1.begin(), innerProdVecFloat1.end(), innerProdVecFloat2.begin(), 0.0f);
        benchmark::DoNotOptimize (prod);
    }
}
BENCHMARK (InnerProdSTLFloat);

static void InnerProdChowFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        auto prod = chowdsp::FloatVectorOperations::innerProduct (innerProdVecFloat1.data(), innerProdVecFloat2.data(), innerProdN);
        benchmark::DoNotOptimize (prod);
    }
}
BENCHMARK (InnerProdChowFloat);

/** Find Absolute Maximum benchmarks............................ */
constexpr int absMaxN = 50000;
const auto absMaxVecFloat = bench_utils::makeRandomVector<float> (absMaxN);

static void AbsMaxSTLFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        auto max = std::abs (*std::max_element (absMaxVecFloat.begin(), absMaxVecFloat.end(), [] (auto a, auto b)
                                                { return std::abs (a) < std::abs (b); }));
        benchmark::DoNotOptimize (max);
    }
}
BENCHMARK (AbsMaxSTLFloat);

static void AbsMaxChowFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        auto max = chowdsp::FloatVectorOperations::findAbsoluteMaximum (absMaxVecFloat.data(), absMaxN);
        benchmark::DoNotOptimize (max);
    }
}
BENCHMARK (AbsMaxChowFloat);

/** RMS benchmarks............................ */
constexpr int rmsN = 10000;
const auto rmsVecFloat = bench_utils::makeRandomVector<float> (rmsN);

static void RMSSTLFloat (benchmark::State& state)
{
    auto idealRMS = [] (const auto* data, int numSamples)
    {
        float squareSum = 0.0f;
        for (int i = 0; i < numSamples; ++i)
            squareSum += data[i] * data[i];
        return std::sqrt (squareSum / (float) numSamples);
    };

    for (auto _ : state)
    {
        auto sum = idealRMS (rmsVecFloat.data(), rmsN);
        benchmark::DoNotOptimize (sum);
    }
}
BENCHMARK (RMSSTLFloat);

static void RMSChowFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        auto sum = chowdsp::FloatVectorOperations::computeRMS (rmsVecFloat.data(), rmsN);
        benchmark::DoNotOptimize (sum);
    }
}
BENCHMARK (RMSChowFloat);

BENCHMARK_MAIN();
