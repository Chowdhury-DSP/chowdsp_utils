#include <TimedUnitTest.h>
#include <chowdsp_filters/chowdsp_filters.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

class BilinearUtilsTest : public TimedUnitTest
{
public:
    BilinearUtilsTest() : TimedUnitTest ("Bilinear Utils Test", "Filters")
    {
    }

    template <int N>
    void testBilinearTransform (const float (&bs)[N], const float (&as)[N], const float (&b_exp)[N], const float (&a_exp)[N], float K)
    {
        float b[N] {};
        float a[N] {};
        chowdsp::Bilinear::BilinearTransform<float, N>::call (b, a, bs, as, K);

        for (int i = 0; i < N; ++i)
        {
            expectWithinAbsoluteError (b[i], b_exp[i], 1.0e-6f, "B coefficient " + juce::String (i) = " is incorrect!");
            expectWithinAbsoluteError (a[i], a_exp[i], 1.0e-6f, "A coefficient " + juce::String (i) = " is incorrect!");
        }
    }

    void runTestTimed() override
    {
        constexpr float fs = 48000.0f;
        const auto kVal = chowdsp::Bilinear::computeKValue (10000.0f, fs);

        beginTest ("First-Order Bilinear Transform");
        testBilinearTransform ({ 1.0f, 2.0f }, { 2.0f, 3.0f }, { 0.500002623f, -0.499981761f }, { 1.f, -0.999968767f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f }, { 2.0f, 3.0f }, { 0.50000304f, -0.499978632f }, { 1.f, -0.999963343f }, kVal);

        beginTest ("Second-Order Bilinear Transform");
        testBilinearTransform ({ 1.0f, 2.0f, 3.0 }, { 2.0f, 3.0f, 4.0f }, { 0.500002623f, -0.999984323f, 0.499981731f }, { 1.f, -1.99996865f, 0.999968647f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f, 3.0 }, { 2.0f, 3.0f, 4.0f }, { 0.50000304f, -0.999981701f, 0.499978632f }, { 1.f, -1.9999634f, 0.999963343f }, kVal);

        beginTest ("Fourth-Order Bilinear Transform");
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }, { 0.500002623f, -1.99998963f, 2.99995327f, -1.99994779f, 0.499981761f }, { 1.f, -3.99996853f, 5.99990654f, -3.9999063f, 0.999968707f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }, { 0.500003099f, -1.99998784f, 2.99994516f, -1.99993896f, 0.499978632f }, { 1.f, -3.99996352f, 5.99989032f, -3.99989009f, 0.999963402f }, kVal);
    }
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

static BilinearUtilsTest bilinearUtilsTest;
