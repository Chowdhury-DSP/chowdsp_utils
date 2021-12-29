#include <TimedUnitTest.h>

class SIMDUtilsTest : public TimedUnitTest
{
public:
    SIMDUtilsTest() : TimedUnitTest ("SIMD Utils Test", "SIMD") {}

    template <typename T>
    void testLoadUnaligned()
    {
        constexpr auto size = dsp::SIMDRegister<T>::size() + 1;
        T testData[size];

        for (size_t i = 0; i < size; ++i)
            testData[i] = (T) i + 1;

        auto reg1 = chowdsp::SIMDUtils::loadUnaligned (testData);
        T sum1 = (T) 0;
        for (size_t i = 0; i < size - 1; ++i)
            sum1 += testData[i];
        expectEquals (sum1, reg1.sum(), "Aligned data is incorrect!");

        auto reg2 = chowdsp::SIMDUtils::loadUnaligned (&testData[1]);
        T sum2 = (T) 0;
        for (size_t i = 1; i < size; ++i)
            sum2 += testData[i];
        expectEquals (sum2, reg2.sum(), "Aligned data is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Float SIMD load unaligned test");
        testLoadUnaligned<float>();

        beginTest ("Double SIMD load unaligned test");
        testLoadUnaligned<double>();
    }
};

static SIMDUtilsTest simdUtilsTest;
