#include <benchmark/benchmark.h>

#include <JuceHeader.h>
#include "bench_utils.h"

namespace
{
constexpr auto sampleRateToUse = 48000.0f;
constexpr int nSamples = 100000;
} // namespace

static void FirstOrderIIR (benchmark::State& state)
{
    chowdsp::FirstOrderLPF<float> lpf;
    lpf.calcCoefs (1000.0f, sampleRateToUse);

    for (auto _ : state)
    {
        lpf.reset();

        float y = 1.0f;
        for (int i = 0; i < nSamples; ++i)
            y = lpf.processSample (y);

        benchmark::DoNotOptimize (y);
    }
}
BENCHMARK (FirstOrderIIR);

static void FirstOrderWDF (benchmark::State& state)
{
    namespace wdft = chowdsp::WDFT;
    wdft::CapacitorT<float> c1 { 1.0e-6f, sampleRateToUse };
    wdft::ResistorT<float> r1 { 10.0e3f };
    auto s1 = wdft::makeSeries<float> (c1, r1);
    auto p1 = wdft::makeInverter<float> (s1);
    wdft::IdealVoltageSourceT<float, decltype (p1)> vs { p1 };

    for (auto _ : state)
    {
        c1.reset();

        float y = 1.0f;
        for (int i = 0; i < nSamples; ++i)
        {
            vs.setVoltage (y);
            vs.incident (p1.reflected());
            p1.incident (vs.reflected());
            y = wdft::voltage<float> (c1);
        }

        benchmark::DoNotOptimize (y);
    }
}
BENCHMARK (FirstOrderWDF);

static void SecondOrderIIR (benchmark::State& state)
{
    chowdsp::SecondOrderLPF<float> lpf;
    lpf.calcCoefs (1000.0f, 0.7071f, sampleRateToUse);

    for (auto _ : state)
    {
        lpf.reset();

        float y = 1.0f;
        for (int i = 0; i < nSamples; ++i)
            y = lpf.processSample (y);

        benchmark::DoNotOptimize (y);
    }
}
BENCHMARK (SecondOrderIIR);

static void SecondOrderWDF (benchmark::State& state)
{
    namespace wdft = chowdsp::WDFT;
    wdft::CapacitorT<float> c1 { 1.0e-6f, sampleRateToUse };
    wdft::CapacitorT<float> c2 { 1.0e-6f, sampleRateToUse };
    wdft::ResistorT<float> r1 { 10.0e3f };
    wdft::ResistorT<float> r2 { 10.0e3f };
    auto s1 = wdft::makeSeries<float> (c1, r1);
    auto p1 = wdft::makeParallel<float> (c2, s1);
    auto s2 = wdft::makeSeries<float> (r2, p1);
    auto i1 = wdft::makeInverter<float> (s2);
    wdft::IdealVoltageSourceT<float, decltype (i1)> vs { i1 };

    for (auto _ : state)
    {
        c1.reset();
        c2.reset();

        float y = 1.0f;
        for (int i = 0; i < nSamples; ++i)
        {
            vs.setVoltage (y);
            vs.incident (p1.reflected());
            p1.incident (vs.reflected());
            y = wdft::voltage<float> (c2);
        }

        benchmark::DoNotOptimize (y);
    }
}
BENCHMARK (SecondOrderWDF);

BENCHMARK_MAIN();
