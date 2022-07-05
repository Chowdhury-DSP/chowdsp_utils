#include <CatchUtils.h>
#include <chowdsp_simd/chowdsp_simd.h>

using namespace chowdsp::SIMDUtils;

#define FLOATFUNC(func) [] (FloatType x) { return func (x); }
#define SIMDFUNC(func) [] (xsimd::batch<FloatType> x) { return func (x); }

TEMPLATE_TEST_CASE ("SIMD Special Math Test", "", float, double)
{
    using FloatType = TestType;

    SECTION ("SIMD Decibels Test")
    {
        static constexpr int nIter = 10;
        static constexpr auto maxErr = []
        {
            if constexpr (std::is_same_v<FloatType, float>)
                return 1.0e-5f;
            else
                return 1.0e-12;
        }();

        std::random_device rd;
        std::mt19937 mt (rd());
        std::uniform_real_distribution<FloatType> minus10To10 ((FloatType) -10, (FloatType) 10);

        auto baseMathTest = [&minus10To10, &mt] (auto floatFunc, auto simdFunc, const std::string& functionName)
        {
            for (int i = 0; i < nIter; ++i)
            {
                auto input = minus10To10 (mt);
                auto expected = floatFunc (input);
                auto actual = simdFunc (xsimd::batch<FloatType> (input));

                for (size_t j = 0; j < xsimd::batch<FloatType>::size; ++j)
                    REQUIRE_MESSAGE (actual.get (j) == Approx (expected).margin (maxErr), "SIMD function is incorrect: " << functionName);
            }
        };

        baseMathTest (FLOATFUNC (gainToDecibels), SIMDFUNC (gainToDecibels), "gainToDecibels");
        baseMathTest (FLOATFUNC (decibelsToGain), SIMDFUNC (decibelsToGain), "decibelsToGain");
    }

    SECTION ("SIMD hmin/hmax test")
    {
        auto refMax = [] (const auto& data)
        { return *std::max_element (data.begin(), data.end()); };

        auto refMin = [] (const auto& data)
        { return *std::min_element (data.begin(), data.end()); };

        auto refAbsMax = [] (const auto& data)
        { return std::abs (*std::max_element (data.begin(), data.end(), [] (auto a, auto b)
                                              { return std::abs (a) < std::abs (b); })); };

        std::random_device rd;
        std::mt19937 mt (rd());
        std::uniform_real_distribution<FloatType> minus1To1 ((FloatType) -1, (FloatType) 1);

        static constexpr int nIter = 10;
        for (int i = 0; i < nIter; ++i)
        {
            using Vec = xsimd::batch<FloatType>;
            std::vector<FloatType> vecData (Vec::size, (FloatType) 0);
            for (auto& v : vecData)
                v = minus1To1 (mt);

            auto vec = xsimd::load_unaligned (vecData.data());
            REQUIRE_MESSAGE (hMinSIMD (vec) == refMin (vecData), "Incorrect minimum value!");
            REQUIRE_MESSAGE (hMaxSIMD (vec) == refMax (vecData), "Incorrect maximum value!");
            REQUIRE_MESSAGE (hAbsMaxSIMD (vec) == refAbsMax (vecData), "Incorrect absolute maximum value!");
        }
    }
}

#undef FLOATFUNC
#undef SIMDFUNC
