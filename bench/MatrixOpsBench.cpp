#include <benchmark/benchmark.h>

#include <chowdsp_math/chowdsp_math.h>

#include "bench_utils.h"

// reference methods copied from: https://github.com/Signalsmith-Audio/reverb-example-code/blob/main/mix-matrix.h
namespace signalsmith
{
// Use like `Householder<double, 8>::inPlace(data)` - size must be ≥ 1
template <typename Sample, int size>
class Householder
{
    static constexpr Sample multiplier { -2.0 / size };

public:
    static void inPlace (Sample* arr)
    {
        Sample sum {};
        for (int i = 0; i < size; ++i)
        {
            sum += arr[i];
        }

        sum *= multiplier;

        for (int i = 0; i < size; ++i)
        {
            arr[i] += sum;
        }
    }
};

// Use like `Hadamard<double, 8>::inPlace(data)` - size must be a power of 2
template <typename Sample, int size>
class Hadamard
{
public:
    static inline void recursiveUnscaled (Sample* data)
    {
        if constexpr (size <= 1)
            return;
        constexpr int hSize = size / 2;

        // Two (unscaled) Hadamards of half the size
        Hadamard<Sample, hSize>::recursiveUnscaled (data);
        Hadamard<Sample, hSize>::recursiveUnscaled (data + hSize);

        // Combine the two halves using sum/difference
        for (int i = 0; i < hSize; ++i)
        {
            Sample a = data[i];
            Sample b = data[i + hSize];
            data[i] = (a + b);
            data[i + hSize] = (a - b);
        }
    }

    static inline void inPlace (Sample* data)
    {
        recursiveUnscaled (data);

        Sample scalingFactor = std::sqrt ((Sample) 1.0 / size);
        for (int c = 0; c < size; ++c)
        {
            data[c] *= scalingFactor;
        }
    }
};
} // namespace signalsmith

constexpr int maxMatSize = 16;
constexpr int N = 25000 * maxMatSize;
auto inputVecFloat = bench_utils::makeRandomVector<float, xsimd::default_allocator<float>> (N);

#define SIGNALSMITH_BENCH_HOUSEHOLDER(order)                                         \
    static void bench_ss_householder_##order (benchmark::State& state)               \
    {                                                                                \
        for (auto _ : state)                                                         \
        {                                                                            \
            for (size_t n = 0; n < N; n += maxMatSize)                               \
                signalsmith::Householder<float, order>::inPlace (&inputVecFloat[n]); \
        }                                                                            \
    }                                                                                \
    BENCHMARK (bench_ss_householder_##order)->MinTime (2);

#define CHOWDSP_BENCH_HOUSEHOLDER(order)                                                    \
    static void bench_chow_householder_##order (benchmark::State& state)                    \
    {                                                                                       \
        for (auto _ : state)                                                                \
        {                                                                                   \
            for (size_t n = 0; n < N; n += maxMatSize)                                      \
                chowdsp::MatrixOps::HouseHolder<float, order>::inPlace (&inputVecFloat[n]); \
        }                                                                                   \
    }                                                                                       \
    BENCHMARK (bench_chow_householder_##order)->MinTime (2);

SIGNALSMITH_BENCH_HOUSEHOLDER (4)
CHOWDSP_BENCH_HOUSEHOLDER (4)
SIGNALSMITH_BENCH_HOUSEHOLDER (8)
CHOWDSP_BENCH_HOUSEHOLDER (8)
SIGNALSMITH_BENCH_HOUSEHOLDER (16)
CHOWDSP_BENCH_HOUSEHOLDER (16)

#define SIGNALSMITH_BENCH_HADAMARD(order)                                         \
    static void bench_ss_hadamard_##order (benchmark::State& state)               \
    {                                                                             \
        for (auto _ : state)                                                      \
        {                                                                         \
            for (size_t n = 0; n < N; n += maxMatSize)                            \
                signalsmith::Hadamard<float, order>::inPlace (&inputVecFloat[n]); \
        }                                                                         \
    }                                                                             \
    BENCHMARK (bench_ss_hadamard_##order)->MinTime (2);

#define CHOWDSP_BENCH_HADAMARD(order)                                                    \
    static void bench_chow_hadamard_##order (benchmark::State& state)                    \
    {                                                                                    \
        for (auto _ : state)                                                             \
        {                                                                                \
            for (size_t n = 0; n < N; n += maxMatSize)                                   \
                chowdsp::MatrixOps::Hadamard<float, order>::inPlace (&inputVecFloat[n]); \
        }                                                                                \
    }                                                                                    \
    BENCHMARK (bench_chow_hadamard_##order)->MinTime (2);

SIGNALSMITH_BENCH_HADAMARD (4)
CHOWDSP_BENCH_HADAMARD (4)
SIGNALSMITH_BENCH_HADAMARD (8)
CHOWDSP_BENCH_HADAMARD (8)
SIGNALSMITH_BENCH_HADAMARD (16)
CHOWDSP_BENCH_HADAMARD (16)

BENCHMARK_MAIN();
