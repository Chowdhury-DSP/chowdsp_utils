#include <TimedUnitTest.h>

using namespace chowdsp::SIMDUtils;

class SIMDLogicTest : public TimedUnitTest
{
public:
    SIMDLogicTest() : TimedUnitTest ("SIMD Logic Test", "SIMD") {}

    template <typename T>
    void selectTest()
    {
        using Vec = typename dsp::SIMDRegister<T>;
        Vec vec {};
        for (size_t i = 0; i < Vec::size(); ++i)
            vec.set (i, i < Vec::size() / 2 ? (T) -1 : (T) 1);

        auto res = select (Vec::greaterThan (vec, (T) 0), (Vec) (T) 100, (Vec) (T) -100);

        for (size_t i = 0; i < Vec::size(); ++i)
        {
            auto actual = res.get (i);
            auto expected = vec.get (i) > (T) 0 ? (T) 100 : (T) -100;

            expectEquals (actual, expected, "Incorrect result at index " + String (i));
        }
    }

    template <typename T>
    void selectIsNANTest()
    {
        using Vec = typename dsp::SIMDRegister<T>;
        Vec vec {};
        for (size_t i = 0; i < Vec::size(); ++i)
            vec.set (i, i < Vec::size() / 2 ? (T) 1 : (T) NAN);

        auto res = select (isnanSIMD (vec), (Vec) (T) 100, (Vec) (T) -100);

        for (size_t i = 0; i < Vec::size(); ++i)
        {
            auto actual = res.get (i);
            auto expected = std::isnan (vec.get (i)) ? (T) 100 : (T) -100;

            expectEquals (actual, expected, "Incorrect result at index " + String (i));
        }
    }

    template <typename T>
    void andNotTest()
    {
        using Vec = typename dsp::SIMDRegister<T>;
        Vec vec {};
        for (size_t i = 0; i < Vec::size(); ++i)
            vec.set (i, (T) i);

        auto res = select (andnot (Vec::greaterThan (vec, (T) 0), Vec::greaterThan (vec, (T) 1)), (Vec) (T) 100, (Vec) (T) -100);

        for (size_t i = 0; i < Vec::size(); ++i)
        {
            auto actual = res.get (i);
            auto expected = ((vec.get (i) > (T) 0) && ! (vec.get (i) > (T) 1)) ? (T) 100 : (T) -100;

            expectEquals (actual, expected, "Incorrect result at index " + String (i));
        }
    }

    void runTestTimed() override
    {
        beginTest ("Select Test");
        selectTest<float>();
        selectTest<double>();

        beginTest ("Select isNAN Test");
        selectIsNANTest<float>();
        selectIsNANTest<double>();

        beginTest ("AND NOT Test");
        andNotTest<float>();
        andNotTest<double>();
    }
};

static SIMDLogicTest simdLogicTest;
