#include <TimedUnitTest.h>

class FloatVectorOperationsTest : public TimedUnitTest
{
public:
    FloatVectorOperationsTest() : TimedUnitTest ("FloatVectorOperations Test")
    {
    }

    template <typename T>
    void accumulateTest (Random& r, Range<int> range)
    {
        auto numValues = r.nextInt (range);
        std::vector<T> values ((size_t) numValues, (T) 0);

        for (auto& v : values)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        constexpr auto maxErr = (T) 1.0e-3;

        auto actual = chowdsp::FloatVectorOperations::accumulate (values.data(), numValues);
        auto expected = std::accumulate (values.begin(), values.end(), (T) 0);
        expectWithinAbsoluteError (actual, expected, maxErr, "Aligned accumulate is incorrect!");

        actual = chowdsp::FloatVectorOperations::accumulate (values.data() + 1, numValues - 1);
        expected = std::accumulate (values.begin() + 1, values.end(), (T) 0);
        expectWithinAbsoluteError (actual, expected, maxErr, "Unaligned accumulate is incorrect!");
    }

    template <typename T>
    void innerProdTest (Random& r, Range<int> range)
    {
        auto numValues = r.nextInt (range);
        std::vector<T> values1 ((size_t) numValues, (T) 0);
        std::vector<T> values2 ((size_t) numValues, (T) 0);

        for (auto& v : values1)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        for (auto& v : values2)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        constexpr auto maxErr = (T) 1.0e-3;

        auto actual = chowdsp::FloatVectorOperations::innerProduct (values1.data(), values2.data(), numValues);
        auto expected = std::inner_product (values1.begin(), values1.end(), values2.begin(), (T) 0);
        expectWithinAbsoluteError (actual, expected, maxErr, "Aligned innerProduct is incorrect!");

        actual = chowdsp::FloatVectorOperations::innerProduct (values1.data() + 1, values2.data(), numValues - 1);
        expected = std::inner_product (values1.begin() + 1, values1.end(), values2.begin(), (T) 0);
        expectWithinAbsoluteError (actual, expected, maxErr, "Unaligned/Aligned innerProduct is incorrect!");

        actual = chowdsp::FloatVectorOperations::innerProduct (values1.data(), values2.data() + 1, numValues - 1);
        expected = std::inner_product (values1.begin(), values1.end() - 1, values2.begin() + 1, (T) 0);
        expectWithinAbsoluteError (actual, expected, maxErr, "Aligned/Unaligned innerProduct is incorrect!");

        actual = chowdsp::FloatVectorOperations::innerProduct (values1.data() + 1, values2.data() + 1, numValues - 1);
        expected = std::inner_product (values1.begin() + 1, values1.end(), values2.begin() + 1, (T) 0);
        expectWithinAbsoluteError (actual, expected, maxErr, "Unaligned innerProduct is incorrect!");
    }

    template <typename T>
    void absMaxTest (Random& r, Range<int> range)
    {
        auto refAbsMax = [] (const auto& begin, const auto end)
        { return std::abs (*std::max_element (begin, end, [] (auto a, auto b)
                                              { return std::abs (a) < std::abs (b); })); };

        auto numValues = r.nextInt (range);
        std::vector<T> values ((size_t) numValues, (T) 0);

        for (auto& v : values)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        constexpr auto maxErr = (T) 1.0e-3;

        auto actual = chowdsp::FloatVectorOperations::findAbsoluteMaximum (values.data(), numValues);
        auto expected = refAbsMax (values.begin(), values.end());
        expectWithinAbsoluteError (actual, expected, maxErr, "Aligned absolute maximum is incorrect!");

        actual = chowdsp::FloatVectorOperations::findAbsoluteMaximum (values.data() + 1, numValues - 1);
        expected = refAbsMax (values.begin() + 1, values.end());
        expectWithinAbsoluteError (actual, expected, maxErr, "Unaligned absolute maximum is incorrect!");
    }

    template <typename T>
    void integerPowerTest (Random& r, Range<int> range)
    {
        for (int exponent = 0; exponent < 19; ++exponent)
        {
            auto numValues = r.nextInt (range);
            std::vector<T> inValues ((size_t) numValues, (T) 0);
            std::vector<T> expValues ((size_t) numValues, (T) 0);
            std::vector<T> actualValues ((size_t) numValues, (T) 0);

            for (auto& v : inValues)
                v = (T) (r.nextFloat() * 2.0f - 1.0f);

            constexpr auto maxErr = (T) 1.0e-6;
            std::transform (inValues.begin(), inValues.end(), expValues.begin(), [exponent] (auto x)
                            { return std::pow (x, (T) exponent); });

            {
                chowdsp::FloatVectorOperations::integerPower (actualValues.data(), inValues.data(), exponent, numValues);
                for (size_t i = 0; i < (size_t) numValues; ++i)
                    expectWithinAbsoluteError (actualValues[i], expValues[i], maxErr, "Aligned value is incorrect!");
            }

            {
                chowdsp::FloatVectorOperations::integerPower (actualValues.data(), inValues.data() + 1, exponent, numValues - 1);
                for (size_t i = 1; i < (size_t) numValues; ++i)
                    expectWithinAbsoluteError (actualValues[i - 1], expValues[i], maxErr, "Aligned value is incorrect!");
            }

            {
                chowdsp::FloatVectorOperations::integerPower (actualValues.data() + 1, inValues.data(), exponent, numValues - 1);
                for (size_t i = 1; i < (size_t) numValues; ++i)
                    expectWithinAbsoluteError (actualValues[i], expValues[i - 1], maxErr, "Aligned value is incorrect!");
            }

            {
                chowdsp::FloatVectorOperations::integerPower (actualValues.data() + 1, inValues.data() + 1, exponent, numValues - 1);
                for (size_t i = 1; i < (size_t) numValues; ++i)
                    expectWithinAbsoluteError (actualValues[i], expValues[i], maxErr, "Aligned value is incorrect!");
            }
        }
    }

    void runTestTimed() override
    {
        if (chowdsp::FloatVectorOperations::isUsingVDSP())
            std::cout << "chowdsp::FloatVectorOperations: using vDSP" << std::endl;
        else
            std::cout << "chowdsp::FloatVectorOperations: not using vDSP" << std::endl;

        auto rand = getRandom();

        beginTest ("Accumulate Test");
        accumulateTest<float> (rand, { 2, 6 });
        accumulateTest<float> (rand, { 100, 200 });
        accumulateTest<float> (rand, { 113, 114 });
        accumulateTest<double> (rand, { 2, 4 });
        accumulateTest<double> (rand, { 100, 200 });
        accumulateTest<double> (rand, { 113, 114 });

        beginTest ("Inner Product Test");
        innerProdTest<float> (rand, { 2, 6 });
        innerProdTest<float> (rand, { 100, 200 });
        innerProdTest<float> (rand, { 113, 114 });
        innerProdTest<double> (rand, { 2, 4 });
        innerProdTest<double> (rand, { 100, 200 });
        innerProdTest<double> (rand, { 113, 114 });

        beginTest ("Absolute Maximum Test");
        absMaxTest<float> (rand, { 2, 6 });
        absMaxTest<float> (rand, { 100, 200 });
        absMaxTest<float> (rand, { 113, 114 });
        absMaxTest<double> (rand, { 2, 4 });
        absMaxTest<double> (rand, { 100, 200 });
        absMaxTest<double> (rand, { 113, 114 });

        beginTest ("Integer Power Test");
        integerPowerTest<float> (rand, { 2, 6 });
        integerPowerTest<float> (rand, { 100, 200 });
        integerPowerTest<float> (rand, { 113, 114 });
        integerPowerTest<double> (rand, { 2, 4 });
        integerPowerTest<double> (rand, { 100, 200 });
        integerPowerTest<double> (rand, { 113, 114 });
    }
};

static FloatVectorOperationsTest floatVectorOperationsTest;
