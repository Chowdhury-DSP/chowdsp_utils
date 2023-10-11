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
    BENCHMARK (name)->MinTime (5)

#define TRIG_BENCH(name, op_mpi_pi, op_full_range)            \
    TRIG_BENCH_RANGE (name##_mpi_pi, data_mpi_pi, op_mpi_pi); \
    TRIG_BENCH_RANGE (name##_m10pi_10pi, data_m10pi_10pi, op_full_range)

#define SIN_BENCH(name, op_mpi_pi, op_full_range) \
    TRIG_BENCH (                                  \
        name##_sin, { \
                            const auto s = op_mpi_pi (x); \
                            benchmark::DoNotOptimize (s); }, { \
                            const auto s = op_full_range (x); \
                            benchmark::DoNotOptimize (s); })

SIN_BENCH (std, std::sin, std::sin);
SIN_BENCH (juce, juce::dsp::FastMathApproximations::sin, std::sin); // JUCE approximation is range-limited
SIN_BENCH (bhaskara, chowdsp::TrigApprox::sin_bhaskara_mpi_pi, chowdsp::TrigApprox::sin_bhaskara);
SIN_BENCH (order1, chowdsp::TrigApprox::sin_1st_order_mpi_pi, chowdsp::TrigApprox::sin_1st_order);
SIN_BENCH (tri_angle_9, chowdsp::TrigApprox::sin_3angle_mpi_pi<9>, chowdsp::TrigApprox::sin_3angle<9>);
SIN_BENCH (tri_angle_7, chowdsp::TrigApprox::sin_3angle_mpi_pi<7>, chowdsp::TrigApprox::sin_3angle<7>);
SIN_BENCH (tri_angle_5, chowdsp::TrigApprox::sin_3angle_mpi_pi<5>, chowdsp::TrigApprox::sin_3angle<5>);
SIN_BENCH (tri_angle_3, chowdsp::TrigApprox::sin_3angle_mpi_pi<3>, chowdsp::TrigApprox::sin_3angle<3>);

#define COS_BENCH(name, op_mpi_pi, op_full_range) \
    TRIG_BENCH (                                  \
        name##_cos, { \
                            const auto c = op_mpi_pi (x); \
                            benchmark::DoNotOptimize (c); }, { \
                            const auto c = op_full_range (x); \
                            benchmark::DoNotOptimize (c); })

COS_BENCH (std, std::cos, std::cos);
COS_BENCH (juce, juce::dsp::FastMathApproximations::cos, std::cos); // JUCE approximation is range-limited
COS_BENCH (bhaskara, chowdsp::TrigApprox::cos_bhaskara_mpi_pi, chowdsp::TrigApprox::cos_bhaskara);
COS_BENCH (order1, chowdsp::TrigApprox::cos_1st_order_mpi_pi, chowdsp::TrigApprox::cos_1st_order);
COS_BENCH (tri_angle_8, chowdsp::TrigApprox::cos_3angle_mpi_pi<8>, chowdsp::TrigApprox::cos_3angle<8>);
COS_BENCH (tri_angle_6, chowdsp::TrigApprox::cos_3angle_mpi_pi<6>, chowdsp::TrigApprox::cos_3angle<6>);
COS_BENCH (tri_angle_4, chowdsp::TrigApprox::cos_3angle_mpi_pi<4>, chowdsp::TrigApprox::cos_3angle<4>);

#define SIN_COS_BENCH(name, op_mpi_pi, op_full_range) \
    TRIG_BENCH (                                      \
        name##_sin_cos, { \
            const auto sc = op_mpi_pi (x); \
            benchmark::DoNotOptimize (sc); }, { \
            const auto sc = op_full_range (x); \
            benchmark::DoNotOptimize (sc); })

const auto std_sin_cos = [] (float x)
{
    return std::make_tuple (std::sin (x), std::cos (x));
};

SIN_COS_BENCH (std, std_sin_cos, std_sin_cos);
SIN_COS_BENCH (tri_angle_7_8, (chowdsp::TrigApprox::sin_cos_3angle_mpi_pi<7, 8>), (chowdsp::TrigApprox::sin_cos_3angle<7, 8>) );
SIN_COS_BENCH (tri_angle_5_6, (chowdsp::TrigApprox::sin_cos_3angle_mpi_pi<5, 6>), (chowdsp::TrigApprox::sin_cos_3angle<5, 6>) );
SIN_COS_BENCH (tri_angle_3_4, (chowdsp::TrigApprox::sin_cos_3angle_mpi_pi<3, 4>), (chowdsp::TrigApprox::sin_cos_3angle<3, 4>) );

BENCHMARK_MAIN();
