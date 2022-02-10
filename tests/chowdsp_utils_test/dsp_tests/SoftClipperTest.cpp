#include <TimedUnitTest.h>

namespace
{
constexpr int N = 1000;
constexpr float maxErr = 1.0e-6f;

float getRandValue (Random& r)
{
    return (r.nextFloat() - 0.5f) * 20.0f;
}

float idealSoftClipper (float x, int degree)
{
    const auto normFactor = float (degree - 1) / (float) degree;
    x *= normFactor;
    if (x > 1.0f)
        return 1.0f;

    if (x < -1.0f)
        return -1.0f;

    return (x - std::pow (x, (float) degree) / (float) degree) / normFactor;
}
} // namespace

class SoftClipperTest : public TimedUnitTest
{
public:
    SoftClipperTest() : TimedUnitTest ("Soft Clipper Test")
    {
    }

    template <int degree>
    void scalarProcessTest (Random& r)
    {
        for (int i = 0; i < N; ++i)
        {
            const auto testX = getRandValue (r);
            const auto expY = idealSoftClipper (testX, degree);
            const auto actualY = chowdsp::SoftClipper<degree>::processSample (testX);
            expectWithinAbsoluteError (actualY, expY, maxErr, "Soft Clipper value at degree " + String (degree) + " is incorrect!");
        }
    }

    template <int degree>
    void vectorProcessTest (Random& r, bool inPlace, bool isBypassed = false)
    {
        AudioBuffer<float> testBuffer (1, N);
        float expYs[N];
        for (int i = 0; i < N; ++i)
        {
            const auto testX = getRandValue (r);
            testBuffer.setSample (0, i, testX);

            if (isBypassed)
                expYs[i] = testX;
            else
                expYs[i] = idealSoftClipper (testX, degree);
        }

        chowdsp::SoftClipper<degree> clipper;
        clipper.prepare ({ 48000.0, (uint32) N, 1 });

        AudioBuffer<float> outBuffer (1, N);
        if (inPlace)
        {
            auto&& testBlock = dsp::AudioBlock<float> { testBuffer };
            auto&& context = dsp::ProcessContextReplacing<float> { testBlock };
            context.isBypassed = isBypassed;
            clipper.process (context);
        }
        else
        {
            auto&& testBlock = dsp::AudioBlock<float> { testBuffer };
            auto&& outBlock = dsp::AudioBlock<float> { outBuffer };

            auto&& context = dsp::ProcessContextNonReplacing<float> { testBlock, outBlock };
            context.isBypassed = isBypassed;
            clipper.process (context);
        }

        for (int i = 0; i < N; ++i)
        {
            float actualY;
            if (inPlace)
                actualY = testBuffer.getSample (0, i);
            else
                actualY = outBuffer.getSample (0, i);

            expectWithinAbsoluteError (actualY, expYs[i], maxErr, "Soft Clipper value at degree " + String (degree) + " is incorrect!");
        }
    }

    void runTestTimed() override
    {
        auto&& rand = getRandom();

        beginTest ("Scalar Process Test");
        scalarProcessTest<5> (rand);
        scalarProcessTest<11> (rand);

        beginTest ("Vector Process Test");
        vectorProcessTest<5> (rand, false);
        vectorProcessTest<11> (rand, true);

        beginTest ("Bypassed Test");
        vectorProcessTest<5> (rand, false, true);
        vectorProcessTest<11> (rand, true, true);
    }
};

static SoftClipperTest softClipperTest;
