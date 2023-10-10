#include <benchmark/benchmark.h>

#include <chowdsp_math/chowdsp_math.h>
#include <juce_dsp/juce_dsp.h>

static constexpr size_t N = 2048;
static constexpr auto data_mpi_pi = []
{
    std::array<float, N> angle_values {};
    for (size_t i = 0; i < N; ++i)
        angle_values[i] = -juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi * (float) i / float (N - 1);
    return angle_values;
}();

static constexpr auto data_m10pi_10pi = []
{
    std::array<float, N> angle_values {};
    for (size_t i = 0; i < N; ++i)
        angle_values[i] = -10.0f * (juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi * (float) i / float (N - 1));
    return angle_values;
}();

#define TRIG_BENCH_RANGE(name, range, op)      \
    static void name (benchmark::State& state) \
    {                                          \
        for (auto _ : state)                   \
        {                                      \
            for (const auto& x : range)        \
                op                             \
        }                                      \
    }                                          \
    BENCHMARK (name)->MinTime (1)

#define TRIG_BENCH(name, op)                  \
    TRIG_BENCH_RANGE (name ## _mpi_pi, data_mpi_pi, op); \
    TRIG_BENCH_RANGE (name ## _m10pi_10pi, data_m10pi_10pi, op)

#define SIN_BENCH(name, op) \
    TRIG_BENCH(name ## _sin, { \
                            const auto s = op (x); \
                            benchmark::DoNotOptimize (s);\
                            })

SIN_BENCH (std, std::sin);
SIN_BENCH (juce, juce::dsp::FastMathApproximations::sin);

BENCHMARK_MAIN();
