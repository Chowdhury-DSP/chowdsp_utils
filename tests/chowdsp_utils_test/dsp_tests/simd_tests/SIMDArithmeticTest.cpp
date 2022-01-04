#include <TimedUnitTest.h>

class SIMDArithmeticTest : public TimedUnitTest
{
public:
    SIMDArithmeticTest() : TimedUnitTest ("SIMD Arithmetic Test", "SIMD") {}

    template <typename T>
    void testDivide()
    {
        dsp::SIMDRegister<T> six ((T) 6);
        dsp::SIMDRegister<T> two ((T) 2);

        using namespace chowdsp::SIMDUtils;
        auto three = six / two;

        expect (three.get (0) == (T) 3, "Divide is incorrect!");
    }

    template <typename T>
    void testArithmetic()
    {
        constexpr auto six = (T) 6;
        dsp::SIMDRegister<T> two ((T) 2);

        using namespace chowdsp::SIMDUtils;

        // addition
        {
            auto eight = six + two;
            expect (eight.get (0) == (T) 8, "Addition is incorrect!");
        }

        // subtraction
        {
            auto four = six - two;
            expect (four.get (0) == (T) 4, "Subtraction is incorrect!");
        }

        // multiplication
        {
            auto twelve = six * two;
            expect (twelve.get (0) == (T) 12, "Multiplication is incorrect!");
        }

        // division
        {
            auto three = six / two;
            expect (three.get (0) == (T) 3, "Division is incorrect!");
        }

        // unary minus
        {
            auto negTwo = -two;
            expect (negTwo.get (0) == (T) -2, "Negation is incorrect!");
        }
    }

    void runTestTimed() override
    {
        beginTest ("Float Divide Test");
        testDivide<float>();

        beginTest ("Double Divide Test");
        testDivide<double>();

        beginTest ("Float Arithmetic Test");
        testArithmetic<float>();

        beginTest ("Double Arithmetic Test");
        testArithmetic<double>();
    }
};

static SIMDArithmeticTest simdArithmeticTest;
