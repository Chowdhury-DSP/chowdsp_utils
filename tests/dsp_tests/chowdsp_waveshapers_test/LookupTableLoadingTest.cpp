#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

struct SlowWaveshaper : public chowdsp::ADAAWaveshaper<float>
{
    explicit SlowWaveshaper (chowdsp::LookupTableCache* lutCache) : chowdsp::ADAAWaveshaper<float> (lutCache, "slow_waveshaper")
    {
        using namespace std::chrono_literals;
        this->initialise (
            [] (double x)
            {
                std::this_thread::sleep_for (1ns);
                return x;
            },
            [] (double x)
            {
                std::this_thread::sleep_for (1ns);
                return x;
            },
            [] (double x)
            {
                std::this_thread::sleep_for (1ns);
                return x;
            });
    }
};

TEST_CASE ("Lookup Table Loading Test", "[dsp][data-structures][waveshapers]")
{
    static constexpr int numInstances = 20;
    chowdsp::LookupTableCache lutCache;

    std::vector<std::unique_ptr<SlowWaveshaper>> waveshapers;
    for (int i = 0; i < numInstances; ++i)
        waveshapers.push_back (std::make_unique<SlowWaveshaper> (&lutCache));

    for (auto& shaper : waveshapers)
    {
        shaper->prepare (1);
        shaper->processSample (0.0f);
    }
}
