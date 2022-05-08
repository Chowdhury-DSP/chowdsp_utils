#include <TimedUnitTest.h>
#include <chowdsp_math/chowdsp_math.h>

class FloatVectorOperationsTest : public TimedUnitTest
{
public:
    FloatVectorOperationsTest() : TimedUnitTest ("FloatVectorOperations Test")
    {
    }

    template <typename T, typename VectorOp, typename ReferenceOp>
    void testReduce1dOp (std::vector<T>& in, VectorOp&& vectorOp, ReferenceOp&& referenceOp, T maxError)
    {
        const int numValues = (int) in.size();

        {
            auto actual = vectorOp (in.data(), numValues);
            auto expected = referenceOp (in.data(), numValues);
            expectWithinAbsoluteError (actual, expected, maxError, "Aligned result is incorrect!");
        }

        {
            auto actual = vectorOp (in.data() + 1, numValues - 1);
            auto expected = referenceOp (in.data() + 1, numValues - 1);
            expectWithinAbsoluteError (actual, expected, maxError, "Unaligned result is incorrect!");
        }
    }

    template <typename T, typename VectorOp, typename ReferenceOp>
    void testReduce2dOp (std::vector<T>& in1, std::vector<T> in2, VectorOp&& vectorOp, ReferenceOp&& referenceOp, T maxError)
    {
        const int numValues = (int) in1.size();

        {
            auto actual = vectorOp (in1.data(), in2.data(), numValues);
            auto expected = referenceOp (in1.data(), in2.data(), numValues);
            expectWithinAbsoluteError (actual, expected, maxError, "Aligned result is incorrect!");
        }

        {
            auto actual = vectorOp (in1.data() + 1, in2.data(), numValues - 1);
            auto expected = referenceOp (in1.data() + 1, in2.data(), numValues - 1);
            expectWithinAbsoluteError (actual, expected, maxError, "Unaligned/Aligned result is incorrect!");
        }

        {
            auto actual = vectorOp (in1.data(), in2.data() + 1, numValues - 1);
            auto expected = referenceOp (in1.data(), in2.data() + 1, numValues - 1);
            expectWithinAbsoluteError (actual, expected, maxError, "Aligned/Unaligned result is incorrect!");
        }

        {
            auto actual = vectorOp (in1.data() + 1, in2.data() + 1, numValues - 1);
            auto expected = referenceOp (in1.data() + 1, in2.data() + 1, numValues - 1);
            expectWithinAbsoluteError (actual, expected, maxError, "Unaligned result is incorrect!");
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
                expectWithinAbsoluteError (actual[i], referenceOp (in[i]), maxError, "Result with aligned inputs and ouputs is incorrect!");
        }

        {
            vectorOp (actual.data(), in.data() + 1, numValues - 1);
            for (size_t i = 1; i < (size_t) numValues; ++i)
                expectWithinAbsoluteError (actual[i - 1], referenceOp (in[i]), maxError, "Result with unaligned input is incorrect!");
        }

        {
            vectorOp (actual.data() + 1, in.data(), numValues - 1);
            for (size_t i = 1; i < (size_t) numValues; ++i)
                expectWithinAbsoluteError (actual[i], referenceOp (in[i - 1]), maxError, "Result with unaligned output is incorrect!");
        }

        {
            vectorOp (actual.data() + 1, in.data() + 1, numValues - 1);
            for (size_t i = 1; i < (size_t) numValues; ++i)
                expectWithinAbsoluteError (actual[i], referenceOp (in[i]), maxError, "Result with unaligned input and output is incorrect!");
        }
    }

    template <typename T, typename VectorOp, typename ReferenceOp>
    void testBinaryOp (std::vector<T>& in1, std::vector<T>& in2, VectorOp&& vectorOp, ReferenceOp&& referenceOp, T maxError)
    {
        std::vector<T> actual (in1.size() + 1);
        const int numValues = (int) in1.size();

        for (int resultOffset = 0; resultOffset < 2; ++resultOffset)
        {
            juce::String outputStr = (resultOffset == 0) ? "Aligned Output: " : "Unaligned Output: ";
            auto* actualPtr = actual.data() + resultOffset;

            {
                vectorOp (actualPtr, in1.data(), in2.data(), numValues);
                for (size_t i = 0; i < (size_t) numValues; ++i)
                    expectWithinAbsoluteError (actualPtr[i], referenceOp (in1[i], in2[i]), maxError, outputStr + "Result with aligned inputs is incorrect!");
            }

            {
                vectorOp (actualPtr, in1.data() + 1, in2.data(), numValues - 1);
                for (size_t i = 1; i < (size_t) numValues; ++i)
                    expectWithinAbsoluteError (actualPtr[i - 1], referenceOp (in1[i], in2[i - 1]), maxError, outputStr + "Result with first unaligned input is incorrect!");
            }

            {
                vectorOp (actualPtr, in1.data(), in2.data() + 1, numValues - 1);
                for (size_t i = 1; i < (size_t) numValues; ++i)
                    expectWithinAbsoluteError (actualPtr[i - 1], referenceOp (in1[i - 1], in2[i]), maxError, outputStr + "Result with second unaligned input is incorrect!");
            }

            {
                vectorOp (actualPtr, in1.data() + 1, in2.data() + 1, numValues - 1);
                for (size_t i = 1; i < (size_t) numValues; ++i)
                    expectWithinAbsoluteError (actualPtr[i - 1], referenceOp (in1[i], in2[i]), maxError, outputStr + "Result with both unaligned inputs is incorrect!");
            }
        }
    }

    template <typename T>
    void divideScalarTest (juce::Random& r, juce::Range<int> range)
    {
        auto numValues = r.nextInt (range);
        std::vector<T> divisor ((size_t) numValues, (T) 0);

        for (auto& v : divisor)
            v = (T) (r.nextFloat() + 1.0f);

        testUnaryOp (
            divisor,
            [] (auto* dest, auto* src, int N) { chowdsp::FloatVectorOperations::divide (dest, (T) 1, src, N); },
            [] (auto x) { return (T) 1 / x; },
            (T) 1.0e-3);
    }

    template <typename T>
    void divideVectorTest (juce::Random& r, juce::Range<int> range)
    {
        auto numValues = r.nextInt (range);
        std::vector<T> dividend ((size_t) numValues, (T) 0);
        std::vector<T> divisor ((size_t) numValues, (T) 0);

        for (auto& v : dividend)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        for (auto& v : divisor)
            v = (T) (r.nextFloat() + 1.0f);

        testBinaryOp (
            dividend,
            divisor,
            [] (auto* dest, auto* src1, auto* src2, int N) { chowdsp::FloatVectorOperations::divide (dest, src1, src2, N); },
            [] (auto num, auto den) { return num / den; },
            (T) 1.0e-3);
    }

    template <typename T>
    void accumulateTest (juce::Random& r, juce::Range<int> range)
    {
        auto numValues = r.nextInt (range);
        std::vector<T> values ((size_t) numValues, (T) 0);

        for (auto& v : values)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        testReduce1dOp (
            values,
            [] (auto* src, int N) { return chowdsp::FloatVectorOperations::accumulate (src, N); },
            [] (auto* src, int N) { return std::accumulate (src, src + N, (T) 0); },
            (T) 1.0e-3);
    }

    template <typename T>
    void innerProdTest (juce::Random& r, juce::Range<int> range)
    {
        auto numValues = r.nextInt (range);
        std::vector<T> values1 ((size_t) numValues, (T) 0);
        std::vector<T> values2 ((size_t) numValues, (T) 0);

        for (auto& v : values1)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        for (auto& v : values2)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        testReduce2dOp (
            values1,
            values2,
            [] (auto* src1, auto* src2, int N) { return chowdsp::FloatVectorOperations::innerProduct (src1, src2, N); },
            [] (auto* src1, auto* src2, int N) { return std::inner_product (src1, src1 + N, src2, (T) 0); },
            (T) 1.0e-3);
    }

    template <typename T>
    void absMaxTest (juce::Random& r, juce::Range<int> range)
    {
        auto refAbsMax = [] (const auto& begin, const auto end) { return std::abs (*std::max_element (begin, end, [] (auto a, auto b) { return std::abs (a) < std::abs (b); })); };

        auto numValues = r.nextInt (range);
        std::vector<T> values ((size_t) numValues, (T) 0);

        for (auto& v : values)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        testReduce1dOp (
            values,
            [] (auto* src, int N) { return chowdsp::FloatVectorOperations::findAbsoluteMaximum (src, N); },
            [&] (auto* src, int N) { return refAbsMax (src, src + N); },
            (T) 1.0e-3);
    }

    template <typename T>
    void integerPowerTest (juce::Random& r, juce::Range<int> range)
    {
        for (int exponent = 0; exponent < 19; ++exponent)
        {
            auto numValues = r.nextInt (range);
            std::vector<T> inValues ((size_t) numValues, (T) 0);

            for (auto& v : inValues)
                v = (T) (r.nextFloat() * 2.0f - 1.0f);

            testUnaryOp (
                inValues,
                [exponent] (auto* dest, auto* src, int N) { chowdsp::FloatVectorOperations::integerPower (dest, src, exponent, N); },
                [exponent] (auto x) { return std::pow (x, (T) exponent); },
                (T) 1.0e-6);
        }
    }

    template <typename T>
    void computeRMSTest (juce::Random& r, juce::Range<int> range)
    {
        auto idealRMS = [] (const auto* data, int numSamples) {
            T squareSum = (T) 0;
            for (int i = 0; i < numSamples; ++i)
                squareSum += data[i] * data[i];
            return std::sqrt (squareSum / (T) numSamples);
        };

        auto numValues = r.nextInt (range);
        std::vector<T> values ((size_t) numValues, (T) 0);

        for (auto& v : values)
            v = (T) (r.nextFloat() * 2.0f - 1.0f);

        testReduce1dOp (
            values,
            [] (auto* src, int N) { return chowdsp::FloatVectorOperations::computeRMS (src, N); },
            [&] (auto* src, int N) { return idealRMS (src, N); },
            (T) 1.0e-3);
    }

    void runTestTimed() override
    {
        if (chowdsp::FloatVectorOperations::isUsingVDSP())
            std::cout << "chowdsp::FloatVectorOperations: using vDSP" << std::endl;
        else
            std::cout << "chowdsp::FloatVectorOperations: not using vDSP" << std::endl;

        auto rand = getRandom();

        std::vector<juce::Range<int>> floatTestRanges {
            { 2, 6 },
            { 100, 200 },
            { 113, 114 }
        };
        std::vector<juce::Range<int>> doubleTestRanges {
            { 2, 4 },
            { 100, 200 },
            { 113, 114 }
        };

        beginTest ("Divide Scalar Test");
        for (const auto& range : floatTestRanges)
            divideScalarTest<float> (rand, range);
        for (const auto& range : doubleTestRanges)
            divideScalarTest<double> (rand, range);

        beginTest ("Divide Vector Test");
        for (const auto& range : floatTestRanges)
            divideVectorTest<float> (rand, range);
        for (const auto& range : doubleTestRanges)
            divideVectorTest<double> (rand, range);

        beginTest ("Accumulate Test");
        for (const auto& range : floatTestRanges)
            accumulateTest<float> (rand, range);
        for (const auto& range : doubleTestRanges)
            accumulateTest<double> (rand, range);

        beginTest ("Inner Product Test");
        for (const auto& range : floatTestRanges)
            innerProdTest<float> (rand, range);
        for (const auto& range : doubleTestRanges)
            innerProdTest<double> (rand, range);

        beginTest ("Absolute Maximum Test");
        for (const auto& range : floatTestRanges)
            absMaxTest<float> (rand, range);
        for (const auto& range : doubleTestRanges)
            absMaxTest<double> (rand, range);

        beginTest ("Integer Power Test");
        for (const auto& range : floatTestRanges)
            integerPowerTest<float> (rand, range);
        for (const auto& range : doubleTestRanges)
            integerPowerTest<double> (rand, range);

        beginTest ("RMS Test");
        for (const auto& range : floatTestRanges)
            computeRMSTest<float> (rand, range);
        for (const auto& range : doubleTestRanges)
            computeRMSTest<double> (rand, range);
    }
};

static FloatVectorOperationsTest floatVectorOperationsTest;
