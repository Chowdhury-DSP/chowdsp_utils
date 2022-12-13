#include <benchmark/benchmark.h>

#include <juce_dsp/juce_dsp.h>
#include <chowdsp_simd/chowdsp_simd.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#include <chowdsp_math/chowdsp_math.h>

#include "bench_utils.h"

constexpr int N = 50000;
const auto lutVecFloat = bench_utils::makeRandomVector<float> (N);
auto outVecFloat = std::vector<float> (N, 0.0f);
const auto juceLUT = juce::dsp::LookupTableTransform<float> ([] (auto x)
                                                             { return std::tanh (x); },
                                                             -10.0f,
                                                             10.0f,
                                                             1 << 17);
const auto chowLUT = chowdsp::LookupTableTransform<float> ([] (auto x)
                                                           { return std::tanh (x); },
                                                           -10.0f,
                                                           10.0f,
                                                           1 << 17);

static void juceTanhLUT (benchmark::State& state)
{
    for (auto _ : state)
        juceLUT.process (lutVecFloat.data(), outVecFloat.data(), N);
}
BENCHMARK (juceTanhLUT);

static void chowTanhLUT (benchmark::State& state)
{
    for (auto _ : state)
        chowLUT.process (lutVecFloat.data(), outVecFloat.data(), N);
}
BENCHMARK (chowTanhLUT);

static void juceTanhLUTUnchecked (benchmark::State& state)
{
    for (auto _ : state)
        juceLUT.processUnchecked (lutVecFloat.data(), outVecFloat.data(), N);
}
BENCHMARK (juceTanhLUTUnchecked);

static void chowTanhLUTUnckecked (benchmark::State& state)
{
    for (auto _ : state)
        chowLUT.processUnchecked (lutVecFloat.data(), outVecFloat.data(), N);
}
BENCHMARK (chowTanhLUTUnckecked);

BENCHMARK_MAIN();
