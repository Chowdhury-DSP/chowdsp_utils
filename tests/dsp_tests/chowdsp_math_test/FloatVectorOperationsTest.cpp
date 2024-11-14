#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>
#include <numeric>
#include <algorithm>

template <typename T, typename VectorOp, typename ReferenceOp>
void testReduce1dOp (std::vector<T>& in, VectorOp&& vectorOp, ReferenceOp&& referenceOp, T maxError)
{
    const int numValues = (int) in.size();

    {
        auto actual = vectorOp (in.data(), numValues);
        auto expected = referenceOp (in.data(), numValues);
        REQUIRE_MESSAGE (actual == Catch::Approx (expected).margin (maxError), "Aligned result is incorrect!");
    }

    {
        auto actual = vectorOp (in.data() + 1, numValues - 1);
        auto expected = referenceOp (in.data() + 1, numValues - 1);
        REQUIRE_MESSAGE (actual == Catch::Approx (expected).margin (maxError), "Unaligned result is incorrect!");
    }
}

template <typename T, typename VectorOp, typename ReferenceOp>
void testReduce2dOp (std::vector<T>& in1, std::vector<T> in2, VectorOp&& vectorOp, ReferenceOp&& referenceOp, T maxError)
{
    const int numValues = (int) in1.size();

    {
        auto actual = vectorOp (in1.data(), in2.data(), numValues);
        auto expected = referenceOp (in1.data(), in2.data(), numValues);
        REQUIRE_MESSAGE (actual == Catch::Approx (expected).margin (maxError), "Aligned result is incorrect!");
    }

    {
        auto actual = vectorOp (in1.data() + 1, in2.data(), numValues - 1);
        auto expected = referenceOp (in1.data() + 1, in2.data(), numValues - 1);
        REQUIRE_MESSAGE (actual == Catch::Approx (expected).margin (maxError), "Unaligned/Aligned result is incorrect!");
    }

    {
        auto actual = vectorOp (in1.data(), in2.data() + 1, numValues - 1);
        auto expected = referenceOp (in1.data(), in2.data() + 1, numValues - 1);
        REQUIRE_MESSAGE (actual == Catch::Approx (expected).margin (maxError), "Aligned/Unaligned result is incorrect!");
    }

    {
        auto actual = vectorOp (in1.data() + 1, in2.data() + 1, numValues - 1);
        auto expected = referenceOp (in1.data() + 1, in2.data() + 1, numValues - 1);
        REQUIRE_MESSAGE (actual == Catch::Approx (expected).margin (maxError), "Unaligned result is incorrect!");
    }
}

template <typename T, typename VectorOp, typename ReferenceOp>
void testUnaryOp (std::vector<T>& in, VectorOp&& vectorOp, ReferenceOp&& referenceOp, T maxError)
{
    std::vector<T> actual (in.size());
    const int numValues = (int) in.size();

    {
        vectorOp (actual.data(), in.data(), numValues);
        for (size_t i = 0; i < (size_t) numValues; ++i)
            REQUIRE_MESSAGE (actual[i] == Catch::Approx (referenceOp (in[i])).margin (maxError), "Result with aligned inputs and ouputs is incorrect!");
    }

    {
        vectorOp (actual.data(), in.data() + 1, numValues - 1);
        for (size_t i = 1; i < (size_t) numValues; ++i)
            REQUIRE_MESSAGE (actual[i - 1] == Catch::Approx (referenceOp (in[i])).margin (maxError), "Result with unaligned input is incorrect!");
    }

    {
        vectorOp (actual.data() + 1, in.data(), numValues - 1);
        for (size_t i = 1; i < (size_t) numValues; ++i)
            REQUIRE_MESSAGE (actual[i] == Catch::Approx (referenceOp (in[i - 1])).margin (maxError), "Result with unaligned output is incorrect!");
    }

    {
        vectorOp (actual.data() + 1, in.data() + 1, numValues - 1);
        for (size_t i = 1; i < (size_t) numValues; ++i)
            REQUIRE_MESSAGE (actual[i] == Catch::Approx (referenceOp (in[i])).margin (maxError), "Result with unaligned input and output is incorrect!");
    }
}

template <typename T, typename VectorOp, typename ReferenceOp>
void testBinaryOp (std::vector<T>& in1, std::vector<T>& in2, VectorOp&& vectorOp, ReferenceOp&& referenceOp, T maxError)
{
    std::vector<T> actual (in1.size() + 1);
    const int numValues = (int) in1.size();

    for (int resultOffset = 0; resultOffset < 2; ++resultOffset)
    {
        std::string outputStr = (resultOffset == 0) ? "Aligned Output: " : "Unaligned Output: ";
        auto* actualPtr = actual.data() + resultOffset;

        {
            vectorOp (actualPtr, in1.data(), in2.data(), numValues);
            for (size_t i = 0; i < (size_t) numValues; ++i)
                REQUIRE_MESSAGE (actualPtr[i] == Catch::Approx (referenceOp (in1[i], in2[i])).margin (maxError), outputStr << "Result with aligned inputs is incorrect!");
        }

        {
            vectorOp (actualPtr, in1.data() + 1, in2.data(), numValues - 1);
            for (size_t i = 1; i < (size_t) numValues; ++i)
                REQUIRE_MESSAGE (actualPtr[i - 1] == Catch::Approx (referenceOp (in1[i], in2[i - 1])).margin (maxError), outputStr << "Result with first unaligned input is incorrect!");
        }

        {
            vectorOp (actualPtr, in1.data(), in2.data() + 1, numValues - 1);
            for (size_t i = 1; i < (size_t) numValues; ++i)
                REQUIRE_MESSAGE (actualPtr[i - 1] == Catch::Approx (referenceOp (in1[i - 1], in2[i])).margin (maxError), outputStr << "Result with second unaligned input is incorrect!");
        }

        {
            vectorOp (actualPtr, in1.data() + 1, in2.data() + 1, numValues - 1);
            for (size_t i = 1; i < (size_t) numValues; ++i)
                REQUIRE_MESSAGE (actualPtr[i - 1] == Catch::Approx (referenceOp (in1[i], in2[i])).margin (maxError), outputStr << "Result with both unaligned inputs is incorrect!");
        }
    }
}

TEMPLATE_TEST_CASE ("FloatVectorOperations Test", "[dsp][math]", float, double)
{
#if JUCE_MAC
    if (chowdsp::FloatVectorOperations::isUsingVDSP())
        INFO ("chowdsp::FloatVectorOperations: using vDSP");
    else
        INFO ("chowdsp::FloatVectorOperations: not using vDSP");
#endif

    std::mt19937 mt (Catch::Generators::Detail::getSeed());
    std::vector<std::pair<int, int>> testRanges {
        { 2, 6 },
        { 100, 200 },
        { 113, 114 }
    };

    SECTION ("Divide Scalar Test")
    {
        for (auto& range : testRanges)
        {
            std::uniform_int_distribution<int> rand (range.first, range.second);
            const auto numValues = rand (mt);
            std::vector<TestType> divisor ((size_t) numValues, (TestType) 0);

            std::uniform_real_distribution<TestType> floatRand ((TestType) 1, (TestType) 2);
            for (auto& v : divisor)
                v = floatRand (mt);

            testUnaryOp (
                divisor,
                [] (auto* dest, auto* src, int N)
                { chowdsp::FloatVectorOperations::divide (dest, (TestType) 1, src, N); },
                [] (auto x)
                { return (TestType) 1 / x; },
                (TestType) 1.0e-3);
        }
    }

    SECTION ("Divide Vector Test")
    {
        for (auto& range : testRanges)
        {
            std::uniform_int_distribution<int> rand (range.first, range.second);
            const auto numValues = rand (mt);
            std::vector<TestType> dividend ((size_t) numValues, (TestType) 0);
            std::vector<TestType> divisor ((size_t) numValues, (TestType) 0);

            std::uniform_real_distribution<TestType> floatRand ((TestType) 0, (TestType) 1);
            for (auto& v : dividend)
                v = floatRand (mt) * (TestType) 2 - (TestType) 1;

            for (auto& v : divisor)
                v = floatRand (mt) + (TestType) 1;

            testBinaryOp (
                dividend,
                divisor,
                [] (auto* dest, auto* src1, auto* src2, int N)
                { chowdsp::FloatVectorOperations::divide (dest, src1, src2, N); },
                [] (auto num, auto den)
                { return num / den; },
                (TestType) 1.0e-3);
        }
    }

    SECTION ("Accumulate Test")
    {
        for (auto& range : testRanges)
        {
            std::uniform_int_distribution<int> rand (range.first, range.second);
            const auto numValues = rand (mt);
            std::vector<TestType> values ((size_t) numValues, (TestType) 0);

            std::uniform_real_distribution<TestType> floatRand ((TestType) -1, (TestType) 1);
            for (auto& v : values)
                v = floatRand (mt);

            testReduce1dOp (
                values,
                [] (auto* src, int N)
                { return chowdsp::FloatVectorOperations::accumulate (src, N); },
                [] (auto* src, int N)
                { return std::accumulate (src, src + N, (TestType) 0); },
                (TestType) 1.0e-3);
        }
    }

    SECTION ("Inner Product Test")
    {
        for (auto& range : testRanges)
        {
            std::uniform_int_distribution<int> rand (range.first, range.second);
            const auto numValues = rand (mt);
            std::vector<TestType> values1 ((size_t) numValues, (TestType) 0);
            std::vector<TestType> values2 ((size_t) numValues, (TestType) 0);

            std::uniform_real_distribution<TestType> floatRand ((TestType) -1, (TestType) 1);
            for (auto& v : values1)
                v = floatRand (mt);

            for (auto& v : values2)
                v = floatRand (mt);

            testReduce2dOp (
                values1,
                values2,
                [] (auto* src1, auto* src2, int N)
                { return chowdsp::FloatVectorOperations::innerProduct (src1, src2, N); },
                [] (auto* src1, auto* src2, int N)
                { return std::inner_product (src1, src1 + N, src2, (TestType) 0); },
                (TestType) 1.0e-3);
        }
    }

    SECTION ("Absolute Maximum Test")
    {
        for (auto& range : testRanges)
        {
            auto refAbsMax = [] (const auto& begin, const auto end)
            { return std::abs (*std::max_element (begin, end, [] (auto a, auto b)
                                                  { return std::abs (a) < std::abs (b); })); };

            std::uniform_int_distribution<int> rand (range.first, range.second);
            const auto numValues = rand (mt);
            std::vector<TestType> values ((size_t) numValues, {});

            std::uniform_real_distribution<TestType> floatRand ((TestType) -1, (TestType) 1);
            for (auto& v : values)
                v = floatRand (mt);

            testReduce1dOp (
                values,
                [] (auto* src, int N)
                { return chowdsp::FloatVectorOperations::findAbsoluteMaximum (src, N); },
                [&] (auto* src, int N)
                { return refAbsMax (src, src + N); },
                (TestType) 1.0e-3);
        }
    }

    SECTION ("Integer Power Test")
    {
        for (auto& range : testRanges)
        {
            for (int exponent = 0; exponent < 19; ++exponent)
            {
                std::uniform_int_distribution<int> rand (range.first, range.second);
                const auto numValues = rand (mt);
                std::vector<TestType> inValues ((size_t) numValues, {});

                std::uniform_real_distribution<TestType> floatRand ((TestType) -1, (TestType) 1);
                for (auto& v : inValues)
                    v = floatRand (mt);

                testUnaryOp (
                    inValues,
                    [exponent] (auto* dest, auto* src, int N)
                    { chowdsp::FloatVectorOperations::integerPower (dest, src, exponent, N); },
                    [exponent] (auto x)
                    { return std::pow (x, (TestType) exponent); },
                    (TestType) 1.0e-6);
            }
        }
    }

    SECTION ("RMS Test")
    {
        for (auto& range : testRanges)
        {
            auto idealRMS = [] (const auto* data, int numSamples)
            {
                auto squareSum = (TestType) 0;
                for (int i = 0; i < numSamples; ++i)
                    squareSum += data[i] * data[i];
                return std::sqrt (squareSum / (TestType) numSamples);
            };

            std::uniform_int_distribution<int> rand (range.first, range.second);
            const auto numValues = rand (mt);
            std::vector<TestType> values ((size_t) numValues, {});

            std::uniform_real_distribution<TestType> floatRand ((TestType) -1, (TestType) 1);
            for (auto& v : values)
                v = floatRand (mt);

            testReduce1dOp (
                values,
                [] (auto* src, int N)
                { return chowdsp::FloatVectorOperations::computeRMS (src, N); },
                [&] (auto* src, int N)
                { return idealRMS (src, N); },
                (TestType) 1.0e-3);
        }
    }

    SECTION ("Contains NaN Test")
    {
        for (int i = 0; i < 4; ++i)
        {
            for (auto& range : testRanges)
            {
                std::uniform_int_distribution<int> rand (range.first, range.second);
                const auto numValues = rand (mt);
                std::vector<TestType> values ((size_t) numValues, {});

                std::uniform_real_distribution<TestType> floatRand ((TestType) -1, (TestType) 1);
                for (auto& v : values)
                    v = floatRand (mt);

                const auto numNaNs = std::uniform_int_distribution<int> { 0, numValues - 1 }(mt);
                std::vector<size_t> valueIndexes ((size_t) numValues, {});
                std::iota (valueIndexes.begin(), valueIndexes.end(), 0);
                std::shuffle (valueIndexes.begin(), valueIndexes.end(), mt);
                for (int j = 0; j < numNaNs; ++j)
                    values[valueIndexes[(size_t) j]] = std::numeric_limits<TestType>::quiet_NaN();

                REQUIRE (chowdsp::FloatVectorOperations::countInfsAndNaNs (values.data(), numValues) == numNaNs);
            }
        }
    }

    SECTION ("Contains Inf Test")
    {
        for (int i = 0; i < 4; ++i)
        {
            for (auto& range : testRanges)
            {
                std::uniform_int_distribution<int> rand (range.first, range.second);
                const auto numValues = rand (mt);
                std::vector<TestType> values ((size_t) numValues, {});

                std::uniform_real_distribution<TestType> floatRand ((TestType) -1, (TestType) 1);
                for (auto& v : values)
                    v = floatRand (mt);

                const auto numInfs = std::uniform_int_distribution<int> { 0, numValues - 1 }(mt);
                std::vector<size_t> valueIndexes ((size_t) numValues, {});
                std::iota (valueIndexes.begin(), valueIndexes.end(), 0);
                std::shuffle (valueIndexes.begin(), valueIndexes.end(), mt);
                for (int j = 0; j < numInfs; ++j)
                    values[valueIndexes[(size_t) j]] = std::numeric_limits<TestType>::infinity();

                REQUIRE (chowdsp::FloatVectorOperations::countInfsAndNaNs (values.data(), numValues) == numInfs);
            }
        }
    }

#if ! JUCE_LINUX
    SECTION ("Rotate Test")
    {
        std::vector<float> allData {};
        allData.resize (128);

        std::vector<float> allRefData {};
        allRefData.resize (128);

        std::vector<float> scratchData {};
        scratchData.resize (128);

        for (const auto& [numValues, rotate] : chowdsp::zip<std::initializer_list<size_t>, std::initializer_list<int>> ({ 2, 14, 15, 100, 101 }, { 1, 7, 7, 51, 52 }))
        {
            auto data = nonstd::span { allData }.subspan (0, numValues);
            std::iota (data.begin(), data.end(), 0.0f);

            auto refData = nonstd::span { allRefData }.subspan (0, numValues);
            std::copy (data.begin(), data.end(), refData.begin());
            std::rotate (refData.begin(), refData.begin() + rotate, refData.end());

            chowdsp::FloatVectorOperations::rotate (data.data(), rotate, (int) numValues, scratchData.data());

            for (const auto& [exp, actual] : chowdsp::zip (refData, data))
                REQUIRE (juce::approximatelyEqual (actual, exp));
        }
    }
#endif
}
