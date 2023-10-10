#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

namespace ta = chowdsp::TrigApprox;

TEMPLATE_TEST_CASE ("Trig Approx Test", "[dsp][math][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using FloatType = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>;

    SECTION ("Sine")
    {
        const auto tester = [] (FloatType (*testFunc) (FloatType), NumericType margin)
        {
            if constexpr (std::is_same_v<NumericType, float>)
                margin = std::sqrt (margin);

            static constexpr auto pi10 = (NumericType) 10 * juce::MathConstants<NumericType>::pi;
            static constexpr auto inc = (NumericType) 0.001 * juce::MathConstants<NumericType>::pi;
            for (auto x = -pi10; x < pi10; x += inc)
            {
                const auto ref = std::sin (x);
                const auto input = (FloatType) x;
                REQUIRE (testFunc (input) == SIMDApprox<FloatType> { ref }.margin (margin));
            }
        };

        tester (&ta::sin_bhaskara, (NumericType) 1.65e-3);
        tester (&ta::sin_1st_order, (NumericType) 5.65e-2);
        tester (&ta::sin_3angle<9, FloatType>, (NumericType) 3.5e-10);
        tester (&ta::sin_3angle<7, FloatType>, (NumericType) 1.15e-7);
        tester (&ta::sin_3angle<5, FloatType>, (NumericType) 4.35e-5);
        tester (&ta::sin_3angle<3, FloatType>, (NumericType) 7.35e-3);
    }

    SECTION ("Cosine")
    {
        const auto tester = [] (FloatType (*testFunc) (FloatType), NumericType margin)
        {
            if constexpr (std::is_same_v<NumericType, float>)
                margin = std::sqrt (margin);

            static constexpr auto pi10 = (NumericType) 10 * juce::MathConstants<NumericType>::pi;
            static constexpr auto inc = (NumericType) 0.001 * juce::MathConstants<NumericType>::pi;
            for (auto x = -pi10; x < pi10; x += inc)
            {
                const auto ref = std::cos (x);
                const auto input = (FloatType) x;
                REQUIRE (testFunc (input) == SIMDApprox<FloatType> { ref }.margin (margin));
            }
        };

        tester (&ta::cos_bhaskara, (NumericType) 1.65e-3);
        tester (&ta::cos_1st_order, (NumericType) 5.65e-2);
        tester (&ta::cos_3angle<8, FloatType>, (NumericType) 8.7e-9);
        tester (&ta::cos_3angle<6, FloatType>, (NumericType) 2.82e-6);
        tester (&ta::cos_3angle<4, FloatType>, (NumericType) 7.93e-4);
    }
}
