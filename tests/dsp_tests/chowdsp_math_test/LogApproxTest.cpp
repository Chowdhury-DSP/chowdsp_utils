#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

namespace la = chowdsp::LogApprox;

TEMPLATE_TEST_CASE ("Log Approx Test", "[dsp][math][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using FloatType = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>;

    SECTION ("Log (base-e)")
    {
        for (auto x = (NumericType) -10; x < (NumericType) 10; x += (NumericType) 0.1)
        {
            const auto x_base = std::exp (x);
            REQUIRE (la::log (x_base) == SIMDApprox<FloatType> { std::log (x_base) }.margin ((NumericType) 0.005));
        }
    }

    SECTION ("Log (base-2)")
    {
        for (auto x = (NumericType) -10; x < (NumericType) 10; x += (NumericType) 0.1)
        {
            const auto x_base = std::pow ((NumericType) 2, x);
            REQUIRE (la::log2 (x_base) == SIMDApprox<FloatType> { std::log2 (x_base) }.margin ((NumericType) 0.01));
        }
    }

    SECTION ("Log (base-10)")
    {
        for (auto x = (NumericType) -10; x < (NumericType) 10; x += (NumericType) 0.1)
        {
            const auto x_base = std::pow ((NumericType) 10, x);
            REQUIRE (la::log10 (x_base) == SIMDApprox<FloatType> { std::log10 (x_base) }.margin ((NumericType) 0.005));
        }
    }
}
