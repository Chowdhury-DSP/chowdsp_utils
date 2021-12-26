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

        auto expected = std::accumulate (values.begin(), values.end(), (T) 0);
        auto actual = chowdsp::FloatVectorOperations::accumulate (values.data(), numValues);
        expectWithinAbsoluteError (actual, expected, maxErr, "Aligned accumulate is incorrect!");

        expected = std::accumulate (values.begin() + 1, values.end(), (T) 0);
        actual = chowdsp::FloatVectorOperations::accumulate (values.data() + 1, numValues - 1);
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

        auto expected = std::inner_product (values1.begin(), values1.end(), values2.begin(), (T) 0);
        auto actual = chowdsp::FloatVectorOperations::innerProduct (values1.data(), values2.data(), numValues);
        expectWithinAbsoluteError (actual, expected, maxErr, "Aligned innerProduct is incorrect!");

        expected = std::inner_product (values1.begin() + 1, values1.end(), values2.begin(), (T) 0);
        actual = chowdsp::FloatVectorOperations::innerProduct (values1.data() + 1, values2.data(), numValues - 1);
        expectWithinAbsoluteError (actual, expected, maxErr, "Unaligned/Aligned innerProduct is incorrect!");

        expected = std::inner_product (values1.begin(), values1.end() - 1, values2.begin() + 1, (T) 0);
        actual = chowdsp::FloatVectorOperations::innerProduct (values1.data(), values2.data() + 1, numValues - 1);
        expectWithinAbsoluteError (actual, expected, maxErr, "Aligned/Unaligned innerProduct is incorrect!");

        expected = std::inner_product (values1.begin() + 1, values1.end(), values2.begin() + 1, (T) 0);
        actual = chowdsp::FloatVectorOperations::innerProduct (values1.data() + 1, values2.data() + 1, numValues - 1);
        expectWithinAbsoluteError (actual, expected, maxErr, "Unaligned innerProduct is incorrect!");
    }

    void runTestTimed() override
    {
        auto rand = getRandom();

        beginTest ("Accumulate Test");
        accumulateTest<float> (rand, { 2, 6 });
        accumulateTest<float> (rand, { 100, 200 });
        accumulateTest<double> (rand, { 2, 6 });
        accumulateTest<double> (rand, { 100, 200 });

        beginTest ("Inner Product Test");
        innerProdTest<float> (rand, { 2, 6 });
        innerProdTest<float> (rand, { 100, 200 });
        innerProdTest<double> (rand, { 2, 6 });
        innerProdTest<double> (rand, { 100, 200 });
    }
};

static FloatVectorOperationsTest floatVectorOperationsTest;
