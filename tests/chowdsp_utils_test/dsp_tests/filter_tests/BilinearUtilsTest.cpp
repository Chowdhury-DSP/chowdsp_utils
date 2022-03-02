#include <TimedUnitTest.h>

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
            expectWithinAbsoluteError (b[i], b_exp[i], 1.0e-6f, "B coefficient " + String (i) = " is incorrect!");
            expectWithinAbsoluteError (a[i], a_exp[i], 1.0e-6f, "A coefficient " + String (i) = " is incorrect!");
        }
    }

    void runTestTimed() override
    {
        constexpr float fs = 48000.0f;
        const auto kVal = chowdsp::Bilinear::computeKValue (10000.0f, fs);

        beginTest ("First-Order Bilinear Transform");
        testBilinearTransform ({ 1.0f, 2.0f }, { 2.0f, 3.0f }, { 0.5000026f, -0.49998177f }, { 1.f, -0.99996875f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f }, { 2.0f, 3.0f }, { 0.50000229f, -0.49998399f }, { 1.f, -0.99997256f }, kVal);

        beginTest ("Second-Order Bilinear Transform");
        testBilinearTransform ({ 1.0f, 2.0f, 3.0 }, { 2.0f, 3.0f, 4.0f }, { 0.5000026f, -0.99998437f, 0.49998177f }, { 1.f, -1.99996875f, 0.99996875f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f, 3.0 }, { 2.0f, 3.0f, 4.0f }, { 0.50000229f, -0.99998628f, 0.49998399f }, { 1.f, -1.99997256f, 0.99997256f }, kVal);

        beginTest ("Fourth-Order Bilinear Transform");
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }, { 0.5000026f, -1.99998958f, 2.99995312f, -1.99994792f, 0.49998177f }, { 1.f, -3.99996875f, 5.99990625f, -3.99990625f, 0.99996875f }, 2 * fs);
        testBilinearTransform ({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }, { 0.50000229f, -1.99999085f, 2.99995884f, -1.99995427f, 0.49998399f }, { 1.f, -3.99997256f, 5.99991768f, -3.99991768f, 0.99997256f }, kVal);
    }
};

static BilinearUtilsTest bilinearUtilsTest;
