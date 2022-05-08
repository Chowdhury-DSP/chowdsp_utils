#include <TimedUnitTest.h>
#include <chowdsp_simd/chowdsp_simd.h>

class SIMDFastMathTest : public TimedUnitTest
{
public:
    SIMDFastMathTest() : TimedUnitTest ("SIMD Fast Math Test", "SIMD") {}

    void testClampPi()
    {
        for (float f = -2132.7f; f < 2132.7f; f += 0.741f)
        {
            auto q = chowdsp::SIMDUtils::clampToPiRangeSIMD (juce::dsp::SIMDRegister<float> (f)).get (0);
            expect (q > -juce::MathConstants<float>::pi, "Clamp returned less than negative PI");
            expect (q < juce::MathConstants<float>::pi, "Clamp returned greater than PI");
        }
    }

    template <typename T>
    void testSinCosSIMD (bool clamp, float rangeMultiplier = 1.0f)
    {
        T squareDev = (T) 0, maxDev = (T) 0;
        constexpr int N = 100000;
        for (int i = 0; i < N; ++i)
        {
            juce::dsp::SIMDRegister<T> p ((T) 0);
            juce::dsp::SIMDRegister<T> cp ((T) 0);
            juce::dsp::SIMDRegister<T> sp ((T) 0);
            for (size_t j = 0; j < juce::dsp::SIMDRegister<T>::size(); ++j)
            {
                auto val = juce::MathConstants<T>::twoPi * (T) getRandom().nextFloat() - juce::MathConstants<T>::pi;
                val *= rangeMultiplier;
                p.set (j, val);
                cp.set (j, std::cos (val));
                sp.set (j, std::sin (val));
            }
            p = clamp ? chowdsp::SIMDUtils::clampToPiRangeSIMD (p) : p;

            auto fcp = chowdsp::SIMDUtils::fastcosSIMD (p);
            auto fsp = chowdsp::SIMDUtils::fastsinSIMD (p);

            for (size_t j = 0; j < juce::dsp::SIMDRegister<T>::size(); ++j)
            {
                T cd = std::abs (cp.get (j) - fcp.get (j));
                T sd = std::abs (sp.get (j) - fsp.get (j));
                if (cd > maxDev)
                    maxDev = cd;
                if (sd > maxDev)
                    maxDev = sd;
                squareDev += cd * cd + sd * sd;
            }
        }
        squareDev = std::sqrt (squareDev) / (T) N;
        expect (maxDev < (T) 1e-4, "Max error is too large!");
        expect (squareDev < (T) 1e-6, "Mean squared error is too large!");
    }

    void runTestTimed() override
    {
        beginTest ("Clamp To Pi Range Test");
        testClampPi();

        beginTest ("Float SIMD fast sin/cos Test (in range)");
        testSinCosSIMD<float> (false);

        beginTest ("Double SIMD fast sin/cos Test (in range)");
        testSinCosSIMD<double> (false);

        beginTest ("Float SIMD fast sin/cos Test (out of range)");
        testSinCosSIMD<float> (true, 10.0f);

        beginTest ("Double SIMD fast sin/cos Test (out of range)");
        testSinCosSIMD<double> (true, 10.0f);

        beginTest ("Float SIMD fast sin/cos Test (way out of range)");
        testSinCosSIMD<float> (true, 100.0f);

        beginTest ("Double SIMD fast sin/cos Test (way out of range)");
        testSinCosSIMD<double> (true, 100.0f);
    }
};

static SIMDFastMathTest simdFastMathTest;
