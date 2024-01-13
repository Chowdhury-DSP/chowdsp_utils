#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

template <int N>
void testBilinearTransform (const float (&bs)[N], const float (&as)[N], const float (&b_exp)[N], const float (&a_exp)[N], float K)
{
    static constexpr int M = N - 1;
    float b[N] {};
    float a[N] {};
    chowdsp::ConformalMaps::Transform<float, M>::bilinear (b, a, bs, as, K);

    for (int i = 0; i < N; ++i)
    {
        REQUIRE_MESSAGE (b[i] == Catch::Approx (b_exp[i]).margin (1.0e-3f), "B coefficient " << std::to_string (i) << " is incorrect!");
        REQUIRE_MESSAGE (a[i] == Catch::Approx (a_exp[i]).margin (1.0e-3f), "A coefficient " << std::to_string (i) << " is incorrect!");
    }
}

TEST_CASE ("Conformal Maps Test", "[dsp][filters]")
{
    constexpr float fs = 48000.0f;
    const auto kVal = chowdsp::ConformalMaps::computeKValue (10000.0f, fs);

    SECTION ("First-Order Bilinear Transform")
    {
        testBilinearTransform ({ 1.0f, 2.0f }, { 2.0f, 3.0f }, { 0.500002623f, -0.499981761f }, { 1.f, -0.999968767f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f }, { 2.0f, 3.0f }, { 0.50000304f, -0.499978632f }, { 1.f, -0.999963343f }, kVal);
    }

    SECTION ("Second-Order Bilinear Transform")
    {
        testBilinearTransform ({ 1.0f, 2.0f, 3.0 }, { 2.0f, 3.0f, 4.0f }, { 0.500002623f, -0.999984323f, 0.499981731f }, { 1.f, -1.99996865f, 0.999968647f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f, 3.0 }, { 2.0f, 3.0f, 4.0f }, { 0.50000304f, -0.999981701f, 0.499978632f }, { 1.f, -1.9999634f, 0.999963343f }, kVal);
    }

    SECTION ("Third-Order Bilinear Transform")
    {
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f }, { 2.0f, 3.0f, 4.0f, 5.0f }, { 0.499997396f, -1.50001302f, 1.50003385f, -0.500018230f }, { 1.0f, -3.00003125f, 3.00006250f, -1.00003125f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f }, { 2.0f, 3.0f, 4.0f, 5.0f }, { 0.499996947f, -1.50001527f, 1.50003969f, -0.500021372f }, { 1.0f, -3.00003664f, 3.00007327f, -1.00003664f }, kVal);
    }

    SECTION ("Fourth-Order Bilinear Transform")
    {
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }, { 0.500002623f, -1.99998963f, 2.99995327f, -1.99994779f, 0.499981761f }, { 1.f, -3.99996853f, 5.99990654f, -3.9999063f, 0.999968707f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }, { 0.500003099f, -1.99998784f, 2.99994516f, -1.99993896f, 0.499978632f }, { 1.f, -3.99996352f, 5.99989032f, -3.99989009f, 0.999963402f }, kVal);
    }

    SECTION ("First-Order Alpha Transform")
    {
        const float bs[2] { 1.0f, 0.0 }; // zero at DC
        const float as[2] { 1.0f, -1.0e10f }; // pole at very high frequency

        // using the alpha transform as a backwards euler transform
        {
            float b[2] {};
            float a[2] {};
            chowdsp::ConformalMaps::Transform<float, 1>::alpha (b, a, bs, as, 2.0f * fs, 0.0f, fs);

            REQUIRE_MESSAGE (juce::exactlyEqual (b[0], -b[1]), "BE should map zero at DC to zero at DC");
            REQUIRE_MESSAGE (std::abs (a[1]) < 0.01f, "BE should map high-frequency pole to near zero");
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
                REQUIRE_MESSAGE (b[i] == Catch::Approx (b_ref[i]).margin (1.0e-6f), "Bilinear Transform should match reference!");
                REQUIRE_MESSAGE (a[i] == Catch::Approx (a_ref[i]).margin (1.0e-6f), "Bilinear Transform should match reference!");
            }
        }
    }

    SECTION ("Second-Order Alpha Transform")
    {
        const float bs[3] { 1.0f, 0.0, 0.0f }; // zero at DC
        const float as[3] { 1.0f, 0.0f, 1.0e10f }; // pole at very high frequency

        // using the alpha transform as a backwards euler transform
        {
            float b[3] {};
            float a[3] {};
            chowdsp::ConformalMaps::Transform<float, 2>::alpha (b, a, bs, as, 2.0f * fs, 0.0f, fs);

            REQUIRE_MESSAGE (std::abs (a[2]) < 1.0f, "BE should damp high-frequency pole");
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
                REQUIRE_MESSAGE (b[i] == Catch::Approx (b_ref[i]).margin (1.0e-6f), "Bilinear Transform should match reference!");
                REQUIRE_MESSAGE (a[i] == Catch::Approx (a_ref[i]).margin (1.0e-6f), "Bilinear Transform should match reference!");
            }
        }
    }
}
