#include <benchmark/benchmark.h>

#include <juce_dsp/juce_dsp.h>
#include <chowdsp_math/chowdsp_math.h>

#include "bench_utils.h"

constexpr int N = 25000;
const auto inputVecFloat = bench_utils::makeRandomVector<float> (N);
auto outVecFloat = std::vector<float> (N, 0.0f);

using namespace chowdsp::Polynomials;

#define POLY_BENCH_7(mode)                                                                                                                            \
    static void bench_Poly7_##mode (benchmark::State& state)                                                                                          \
    {                                                                                                                                                 \
        for (auto _ : state)                                                                                                                          \
        {                                                                                                                                             \
            for (size_t n = 0; n < N; ++n)                                                                                                            \
                outVecFloat[n] = mode<7, float> ({ { 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f }, poly_order_descending {} }, inputVecFloat[n]); \
        }                                                                                                                                             \
    }                                                                                                                                                 \
    BENCHMARK (bench_Poly7_##mode)->MinTime (1);

POLY_BENCH_7 (naive)
POLY_BENCH_7 (estrin)
POLY_BENCH_7 (horner)

#define POLY_BENCH_10(mode)                                                                                                                                              \
    static void bench_Poly10_##mode (benchmark::State& state)                                                                                                            \
    {                                                                                                                                                                    \
        for (auto _ : state)                                                                                                                                             \
        {                                                                                                                                                                \
            for (size_t n = 0; n < N; ++n)                                                                                                                               \
                outVecFloat[n] = mode<10, float> ({ { 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0 }, poly_order_descending {} }, inputVecFloat[n]); \
        }                                                                                                                                                                \
    }                                                                                                                                                                    \
    BENCHMARK (bench_Poly10_##mode)->MinTime (1);

POLY_BENCH_10 (naive)
POLY_BENCH_10 (estrin)
POLY_BENCH_10 (horner)

BENCHMARK_MAIN();
