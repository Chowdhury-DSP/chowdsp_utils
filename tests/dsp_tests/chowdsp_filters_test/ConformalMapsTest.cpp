#include <TimedUnitTest.h>
#include <chowdsp_filters/chowdsp_filters.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

class ConformalMapsTest : public TimedUnitTest
{
public:
    ConformalMapsTest() : TimedUnitTest ("Conformal Maps Test", "Filters")
    {
    }

    template <int N>
    void testBilinearTransform (const float (&bs)[N], const float (&as)[N], const float (&b_exp)[N], const float (&a_exp)[N], float K)
    {
        static constexpr int M = N - 1;
        float b[N] {};
        float a[N] {};
        chowdsp::ConformalMaps::Transform<float, M>::bilinear (b, a, bs, as, K);

        for (int i = 0; i < N; ++i)
        {
            expectWithinAbsoluteError (b[i], b_exp[i], 1.0e-6f, "B coefficient " + juce::String (i) = " is incorrect!");
            expectWithinAbsoluteError (a[i], a_exp[i], 1.0e-6f, "A coefficient " + juce::String (i) = " is incorrect!");
        }
    }

    void firstOrderAlphaTransformTest()
    {
        static constexpr auto fs = 48000.0f;
        const float bs[2] { 1.0f, 0.0 }; // zero at DC
        const float as[2] { 1.0f, -1.0e10f }; // pole at very high frequency

        // using the alpha transform as a backwards euler transform
        {
            float b[2] {};
            float a[2] {};
            chowdsp::ConformalMaps::Transform<float, 1>::alpha (b, a, bs, as, 2.0f * fs, 0.0f, fs);

            expectEquals (b[0], -b[1], "BE should map zero at DC to zero at DC");
            expectLessThan (std::abs (a[1]), 0.01f, "BE should map high-frequency pole to near zero");
        }

        // using the alpha transform as a bilinear transform
        {
            float b[2] {};
            float a[2] {};
            chowdsp::ConformalMaps::Transform<float, 1>::alpha (b, a, bs, as, 2.0f * fs, 1.0f, fs);

            float b_ref[2] {};
            float a_ref[2] {};
            chowdsp::ConformalMaps::Transform<float, 1>::bilinear (b_ref, a_ref, bs, as, 2.0f * fs);

            for (int i = 0; i < 2; ++i)
            {
                expectWithinAbsoluteError (b[i], b_ref[i], 1.0e-6f, "Bilinear Transform should match reference!");
                expectWithinAbsoluteError (a[i], a_ref[i], 1.0e-6f, "Bilinear Transform should match reference!");
            }
        }
    }

    void secondOrderAlphaTransformTest()
    {
        static constexpr auto fs = 48000.0f;
        const float bs[3] { 1.0f, 0.0, 0.0f }; // zero at DC
        const float as[3] { 1.0f, 0.0f, 1.0e10f }; // pole at very high frequency

        // using the alpha transform as a backwards euler transform
        {
            float b[3] {};
            float a[3] {};
            chowdsp::ConformalMaps::Transform<float, 2>::alpha (b, a, bs, as, 2.0f * fs, 0.0f, fs);

            expectLessThan (std::abs (a[2]), 1.0f, "BE should damp high-frequency pole");
        }

        // using the alpha transform as a bilinear transform
        {
            float b[3] {};
            float a[3] {};
            chowdsp::ConformalMaps::Transform<float, 2>::alpha (b, a, bs, as, 2.0f * fs, 1.0f, fs);

            float b_ref[3] {};
            float a_ref[3] {};
            chowdsp::ConformalMaps::Transform<float, 2>::bilinear (b_ref, a_ref, bs, as, 2.0f * fs);

            for (int i = 0; i < 3; ++i)
            {
                expectWithinAbsoluteError (b[i], b_ref[i], 1.0e-6f, "Bilinear Transform should match reference!");
                expectWithinAbsoluteError (a[i], a_ref[i], 1.0e-6f, "Bilinear Transform should match reference!");
            }
        }
    }

    void runTestTimed() override
    {
        constexpr float fs = 48000.0f;
        const auto kVal = chowdsp::ConformalMaps::computeKValue (10000.0f, fs);

        beginTest ("First-Order Bilinear Transform");
        testBilinearTransform ({ 1.0f, 2.0f }, { 2.0f, 3.0f }, { 0.500002623f, -0.499981761f }, { 1.f, -0.999968767f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f }, { 2.0f, 3.0f }, { 0.50000304f, -0.499978632f }, { 1.f, -0.999963343f }, kVal);

        beginTest ("Second-Order Bilinear Transform");
        testBilinearTransform ({ 1.0f, 2.0f, 3.0 }, { 2.0f, 3.0f, 4.0f }, { 0.500002623f, -0.999984323f, 0.499981731f }, { 1.f, -1.99996865f, 0.999968647f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f, 3.0 }, { 2.0f, 3.0f, 4.0f }, { 0.50000304f, -0.999981701f, 0.499978632f }, { 1.f, -1.9999634f, 0.999963343f }, kVal);

        beginTest ("Fourth-Order Bilinear Transform");
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }, { 0.500002623f, -1.99998963f, 2.99995327f, -1.99994779f, 0.499981761f }, { 1.f, -3.99996853f, 5.99990654f, -3.9999063f, 0.999968707f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }, { 0.500003099f, -1.99998784f, 2.99994516f, -1.99993896f, 0.499978632f }, { 1.f, -3.99996352f, 5.99989032f, -3.99989009f, 0.999963402f }, kVal);

        beginTest ("First-Order Alpha Transform");
        firstOrderAlphaTransformTest();

        beginTest ("Second-Order Alpha Transform");
        secondOrderAlphaTransformTest();
    }
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

static ConformalMapsTest conformalMapsTest;
