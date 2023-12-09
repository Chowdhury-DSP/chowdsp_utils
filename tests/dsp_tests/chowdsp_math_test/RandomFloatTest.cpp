#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

TEMPLATE_TEST_CASE ("Random Float Static Test", "[dsp][math][random]", float, double)
{
    using namespace chowdsp::RandomUtils;

    std::array<int, 10> counters {};
    static constexpr int N = 1'000'000;

    using B = BasisTypeT<TestType>;
    B seed = 444;

    SECTION ("Range [0, 1]")
    {
        for (int n = 0; n < N; ++n)
        {
            const auto x = rng_0_1 (seed);
            const auto bin = std::floor (x * (TestType) counters.size());
            counters[(size_t) bin]++;
        }

        for (size_t i = 0; i < counters.size(); ++i)
        {
            const auto pct = (float) counters[i] * (float) counters.size() / (float) N;
            REQUIRE (pct == Catch::Approx { 1.0f }.margin (0.1));
        }
    }

    SECTION ("Range [-1, 1]")
    {
        for (int n = 0; n < N; ++n)
        {
            const auto x = rng_m1_1 (seed);
            const auto bin = std::floor (std::abs (x) * (TestType) counters.size());
            counters[(size_t) bin]++;
        }

        for (size_t i = 0; i < counters.size(); ++i)
        {
            const auto pct = (float) counters[i] * (float) counters.size() / (float) N;
            REQUIRE (pct == Catch::Approx { 1.0f }.margin (0.1));
        }
    }
}

TEMPLATE_TEST_CASE ("Random SIMD Float Static Test", "[dsp][math][random]", float, double)
{
    using namespace chowdsp::RandomUtils;

    std::array<int, 10> counters {};
    static constexpr int N = 1'000'000;

    using T = xsimd::batch<TestType>;
    using B = BasisTypeT<T>;

    B seed {};
    if constexpr (T::size == 4)
        seed = { 441, 442, 443, 444 };
    else if constexpr (T::size == 2)
        seed = { 440, 444 };

    SECTION ("Range [0, 1]")
    {
        for (int n = 0; n < N; ++n)
        {
            const auto x = rng_0_1 (seed);
            for (size_t k = 0; k < T::size; ++k)
            {
                const auto bin = std::floor (x.get (k) * (TestType) counters.size());
                counters[(size_t) bin]++;
            }
        }

        for (size_t i = 0; i < counters.size(); ++i)
        {
            const auto pct = (float) counters[i] * (float) counters.size() / (float) (N * T::size);
            REQUIRE (pct == Catch::Approx { 1.0f }.margin (0.1));
        }
    }

    SECTION ("Range [-1, 1]")
    {
        for (int n = 0; n < N; ++n)
        {
            const auto x = rng_m1_1 (seed);
            for (size_t k = 0; k < T::size; ++k)
            {
                auto bin = std::floor (std::abs (x.get (k)) * (TestType) counters.size());
                counters[(size_t) bin]++;
            }
        }

        for (size_t i = 0; i < counters.size(); ++i)
        {
            const auto pct = (float) counters[i] * (float) counters.size() / (float) (N * T::size);
            REQUIRE (pct == Catch::Approx { 1.0f }.margin (0.1));
        }
    }
}
