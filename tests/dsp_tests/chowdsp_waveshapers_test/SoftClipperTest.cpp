#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

namespace
{
constexpr int N = 1000;
constexpr float maxErr = 1.0e-6f;

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

template <int degree>
void scalarProcessTest()
{
    auto minus10To10 = test_utils::RandomFloatGenerator { -10.0f, 10.0f };
    for (int i = 0; i < N; ++i)
    {
        const auto testX = minus10To10();
        const auto expY = idealSoftClipper (testX, degree);
        const auto actualY = chowdsp::SoftClipper<degree>::processSample (testX);
        REQUIRE_MESSAGE (actualY == Catch::Approx (expY).margin (maxErr), "Soft Clipper value at degree " << std::to_string (degree) << " is incorrect!");
    }
}

template <int degree>
void vectorProcessTest()
{
    auto minus10To10 = test_utils::RandomFloatGenerator { -10.0f, 10.0f };
    chowdsp::Buffer<float> testBuffer (1, N);
    float expYs[N];
    for (int i = 0; i < N; ++i)
    {
        const auto testX = minus10To10();
        testBuffer.getWritePointer (0)[i] = testX;
        expYs[i] = idealSoftClipper (testX, degree);
    }

    chowdsp::SoftClipper<degree> clipper;
    clipper.prepare ({ 48000.0, (juce::uint32) N, 1 });
    clipper.processBlock (testBuffer);

    for (int i = 0; i < N; ++i)
    {
        float actualY = testBuffer.getReadPointer (0)[i];
        REQUIRE_MESSAGE (actualY == Catch::Approx (expYs[i]).margin (maxErr), "Soft Clipper value at degree " << std::to_string (degree) << " is incorrect!");
    }
}

TEST_CASE ("Soft Clipper Test", "[dsp][waveshapers]")
{
    SECTION ("Scalar Process Test")
    {
        scalarProcessTest<5>();
        scalarProcessTest<11>();
    }

    SECTION ("Vector Process Test")
    {
        vectorProcessTest<5>();
        vectorProcessTest<11>();
    }
}
