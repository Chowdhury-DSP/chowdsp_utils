#include <random>

#include <benchmark/benchmark.h>

#include <JuceHeader.h>

template <typename T>
auto makeRandomVector (int num)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine { rnd_device() }; // Generates random integers
    std::normal_distribution<float> dist { (T) 0, (T) 1 };

    std::vector<T> vec ((size_t) num);
    std::generate (vec.begin(), vec.end(), [&dist, &mersenne_engine]()
                   { return dist (mersenne_engine); });

    return std::move (vec);
}

constexpr int accumulateN = 10000;
const auto accumulateVecFloat = makeRandomVector<float> (accumulateN);

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

BENCHMARK_MAIN();
