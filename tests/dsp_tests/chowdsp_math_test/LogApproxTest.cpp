#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

namespace la = chowdsp::LogApprox;

template <typename T>
T pow10 (T x)
{
    return std::pow ((T) 10, x);
}

TEMPLATE_TEST_CASE ("Log Approx Test", "[dsp][math][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using FloatType = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>;

    const auto tester = [] (NumericType (*refFuncExp) (NumericType), NumericType (*refFuncLog) (NumericType), FloatType (*testFunc) (FloatType), NumericType margin)
    {
        for (auto x = (NumericType) -10; x < (NumericType) 10; x += (NumericType) 0.1)
        {
            const auto x_base = refFuncExp (x);
            const auto input = (FloatType) x_base;
            REQUIRE (testFunc (input) == SIMDApprox<FloatType> { refFuncLog (x_base) }.margin (margin));
        }
    };

    SECTION ("Log (base-e)")
    {
        tester (&std::exp, &std::log, &la::log<FloatType, 1>, (NumericType) 0.06);
        tester (&std::exp, &std::log, &la::log<FloatType, 2>, (NumericType) 0.007);
        tester (&std::exp, &std::log, &la::log<FloatType, 3>, (NumericType) 0.005);
    }

    SECTION ("Log (base-2)")
    {
        tester (&std::exp2, &std::log2, &la::log2<FloatType, 1>, (NumericType) 0.09);
        tester (&std::exp2, &std::log2, &la::log2<FloatType, 2>, (NumericType) 0.01);
        tester (&std::exp2, &std::log2, &la::log2<FloatType, 3>, (NumericType) 0.007);
    }

    SECTION ("Log (base-10)")
    {
        tester (&pow10, &std::log10, &la::log10<FloatType, 1>, (NumericType) 0.06);
        tester (&pow10, &std::log10, &la::log10<FloatType, 2>, (NumericType) 0.007);
        tester (&pow10, &std::log10, &la::log10<FloatType, 3>, (NumericType) 0.005);
    }
}
