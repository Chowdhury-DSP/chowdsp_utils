#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

namespace pa = chowdsp::PowApprox;

TEMPLATE_TEST_CASE ("Pow Approx Test", "[dsp][math][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using FloatType = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>;

    SECTION ("Exp")
    {
        for (auto x = (NumericType) -10; x < (NumericType) 10; x += (NumericType) 0.1)
        {
            const auto ref = std::exp (x);
            const auto margin = (NumericType) 0.001 * ref;
            const auto input = (FloatType) x;
            REQUIRE (pa::exp (input) == SIMDApprox<FloatType> { ref }.margin (margin));
        }
    }

    SECTION ("Pow2")
    {
        for (auto x = (NumericType) -10; x < (NumericType) 10; x += (NumericType) 0.1)
        {
            const auto ref = std::pow ((NumericType) 2, x);
            const auto margin = (NumericType) 0.001 * ref;
            const auto input = (FloatType) x;
            REQUIRE (pa::pow2 (input) == SIMDApprox<FloatType> { ref }.margin (margin));
        }
    }

    SECTION ("Pow10")
    {
        for (auto x = (NumericType) -10; x < (NumericType) 10; x += (NumericType) 0.1)
        {
            const auto ref = std::pow ((NumericType) 10, x);
            const auto margin = (NumericType) 0.001 * ref;
            const auto input = (FloatType) x;
            REQUIRE (pa::pow10 (input) == SIMDApprox<FloatType> { ref }.margin (margin));
        }
    }
}
