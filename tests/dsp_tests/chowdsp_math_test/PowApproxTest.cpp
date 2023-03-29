#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

namespace pa = chowdsp::PowApprox;

template <typename T>
T pow10 (T x)
{
    return std::pow ((T) 10, x);
}

TEMPLATE_TEST_CASE ("Pow Approx Test", "[dsp][math][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using FloatType = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>;

    const auto tester = [] (NumericType (*refFunc) (NumericType), FloatType (*testFunc) (FloatType), NumericType marginMultiplier)
    {
        for (auto x = (NumericType) -10; x < (NumericType) 10; x += (NumericType) 0.1)
        {
            const auto ref = refFunc (x);
            const auto margin = marginMultiplier * ref;
            const auto input = (FloatType) x;
            REQUIRE (testFunc (input) == SIMDApprox<FloatType> { ref }.margin (margin));
        }
    };

    SECTION ("Exp")
    {
        tester (&std::exp, &pa::exp<FloatType, 1>, (NumericType) 0.065);
        tester (&std::exp, &pa::exp<FloatType, 2>, (NumericType) 0.004);
        tester (&std::exp, &pa::exp<FloatType, 3>, (NumericType) 0.001);
    }

    SECTION ("Pow2")
    {
        tester (&std::exp2, &pa::pow2<FloatType, 1>, (NumericType) 0.065);
        tester (&std::exp2, &pa::pow2<FloatType, 2>, (NumericType) 0.004);
        tester (&std::exp2, &pa::pow2<FloatType, 3>, (NumericType) 0.001);
    }

    SECTION ("Pow10")
    {
        tester (&pow10, &pa::pow10<FloatType, 1>, (NumericType) 0.065);
        tester (&pow10, &pa::pow10<FloatType, 2>, (NumericType) 0.004);
        tester (&pow10, &pa::pow10<FloatType, 3>, (NumericType) 0.001);
    }
}
