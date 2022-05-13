#include <TimedUnitTest.h>
#include <chowdsp_simd/chowdsp_simd.h>

using namespace chowdsp::SIMDUtils;

class SIMDSpecialMathTest : public TimedUnitTest
{
public:
    SIMDSpecialMathTest() : TimedUnitTest ("SIMD Special Math Test", "SIMD") {}

    template <typename T>
    void hMinMaxTest (int nIter, juce::Random& r)
    {
        auto refMax = [] (const auto& data) { return *std::max_element (data.begin(), data.end()); };

        auto refMin = [] (const auto& data) { return *std::min_element (data.begin(), data.end()); };

        auto refAbsMax = [] (const auto& data) { return std::abs (*std::max_element (data.begin(), data.end(), [] (auto a, auto b) { return std::abs (a) < std::abs (b); })); };

        for (int i = 0; i < nIter; ++i)
        {
            using Vec = xsimd::batch<T>;
            std::vector<T> vecData (Vec::size, (T) 0);
            for (auto& v : vecData)
                v = (T) (r.nextFloat() * 2.0f - 1.0f);

            auto vec = xsimd::load_unaligned (vecData.data());
            expectEquals (hMinSIMD (vec), refMin (vecData), "Incorrect minimum value!");
            expectEquals (hMaxSIMD (vec), refMax (vecData), "Incorrect maximum value!");
            expectEquals (hAbsMaxSIMD (vec), refAbsMax (vecData), "Incorrect absolute maximum value!");
        }
    }

    void runTestTimed() override
    {
        auto rand = getRandom();

        beginTest ("Float SIMD hmin/hmax test");
        hMinMaxTest<float> (10, rand);

        beginTest ("Double SIMD hmin/hmax test");
        hMinMaxTest<double> (10, rand);
    }
};

static SIMDSpecialMathTest simdSpecialMathTest;
