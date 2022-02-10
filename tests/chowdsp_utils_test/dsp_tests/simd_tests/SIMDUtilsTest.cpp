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
        expectEquals (sum2, reg2.sum(), "Unaligned data is incorrect!");
    }

    template <typename T>
    void testStoreUnaligned()
    {
        constexpr auto size = dsp::SIMDRegister<T>::size() + 1;
        T testData[size];

        auto reg1 = dsp::SIMDRegister<T> {};
        for (size_t i = 0; i < size - 1; ++i)
            reg1.set (i, (T) i + 1);

        chowdsp::SIMDUtils::storeUnaligned (testData, reg1);
        T sum1 = (T) 0;
        for (size_t i = 0; i < size - 1; ++i)
            sum1 += testData[i];
        expectEquals (sum1, reg1.sum(), "Aligned data is incorrect!");

        auto reg2 = dsp::SIMDRegister<T> {};
        for (size_t i = 0; i < size - 1; ++i)
            reg2.set (i, (T) i + 2);

        chowdsp::SIMDUtils::storeUnaligned (&testData[1], reg2);
        T sum2 = (T) 0;
        for (size_t i = 1; i < size; ++i)
            sum2 += testData[i];
        expectEquals (sum2, reg2.sum(), "Unaligned data is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Float SIMD load unaligned test");
        testLoadUnaligned<float>();

        beginTest ("Double SIMD load unaligned test");
        testLoadUnaligned<double>();

        beginTest ("Float SIMD store unaligned test");
        testStoreUnaligned<float>();

        beginTest ("Double SIMD store unaligned test");
        testStoreUnaligned<double>();
    }
};

static SIMDUtilsTest simdUtilsTest;
