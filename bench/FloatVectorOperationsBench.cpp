#include <algorithm>
#include <numeric>

#include <benchmark/benchmark.h>

#include <juce_dsp/juce_dsp.h>
#include <chowdsp_simd/chowdsp_simd.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#include <chowdsp_math/chowdsp_math.h>

#include "bench_utils.h"

/** Divide Scalar benchmarks............................ */
constexpr int divScalarN = 100000;
const auto divScalarVecFloat = bench_utils::makeRandomVector<float> (divScalarN);
auto divScalarOutVecFloat = std::vector<float> (divScalarN, 0.0f);

static void DivideScalarSTLFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (divScalarVecFloat.begin(), divScalarVecFloat.end(), divScalarOutVecFloat.begin(), [] (auto x)
                        { return 1.0f / x; });
    }
}
BENCHMARK (DivideScalarSTLFloat);

static void DivideScalarChowFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        chowdsp::FloatVectorOperations::divide (divScalarOutVecFloat.data(), 1.0f, divScalarVecFloat.data(), divScalarN);
    }
}
BENCHMARK (DivideScalarChowFloat);

/** Divide Vector benchmarks............................ */
constexpr int divVectorN = 100000;
const auto divVectorVecFloat1 = bench_utils::makeRandomVector<float> (divVectorN);
const auto divVectorVecFloat2 = bench_utils::makeRandomVector<float> (divVectorN);
auto divVectorOutVecFloat = std::vector<float> (divVectorN, 0.0f);

static void DivideVectorSTLFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        std::transform (divVectorVecFloat1.begin(), divVectorVecFloat1.end(), divVectorVecFloat2.begin(), divVectorOutVecFloat.begin(), [] (auto a, auto b)
                        { return a / b; });
    }
}
BENCHMARK (DivideVectorSTLFloat);

static void DivideVectorChowFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        chowdsp::FloatVectorOperations::divide (divVectorOutVecFloat.data(), divVectorVecFloat1.data(), divVectorVecFloat2.data(), divVectorN);
    }
}
BENCHMARK (DivideVectorChowFloat);

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

/** isInfOrNAN benchmarks............................ */
constexpr int countInfOrNanN = 10000;
const auto countInfOrNanVecFloat = bench_utils::makeRandomVector<float> (countInfOrNanN);

static void countNanSTLFloat (benchmark::State& state)
{
    auto idealCountNan = [] (const auto* data, int numSamples)
    {
        int nanCount = 0;
        for (int i = 0; i < numSamples; ++i)
            nanCount += (int) std::isnan (data[i]);
        return nanCount;
    };

    for (auto _ : state)
    {
        auto nanCount = idealCountNan (countInfOrNanVecFloat.data(), countInfOrNanN);
        benchmark::DoNotOptimize (nanCount);
    }
}
BENCHMARK (countNanSTLFloat);

static void countInfSTLFloat (benchmark::State& state)
{
    auto idealCountInf = [] (const auto* data, int numSamples)
    {
        int infsCount = 0;
        for (int i = 0; i < numSamples; ++i)
            infsCount += (int) std::isinf (data[i]);
        return infsCount;
    };

    for (auto _ : state)
    {
        auto infsCount = idealCountInf (countInfOrNanVecFloat.data(), countInfOrNanN);
        benchmark::DoNotOptimize (infsCount);
    }
}
BENCHMARK (countInfSTLFloat);

static void countInfChowFloat (benchmark::State& state)
{
    for (auto _ : state)
    {
        auto infsCount = chowdsp::FloatVectorOperations::countInfsAndNaNs (countInfOrNanVecFloat.data(), countInfOrNanN);
        benchmark::DoNotOptimize (infsCount);
    }
}
BENCHMARK (countInfChowFloat);

BENCHMARK_MAIN();
