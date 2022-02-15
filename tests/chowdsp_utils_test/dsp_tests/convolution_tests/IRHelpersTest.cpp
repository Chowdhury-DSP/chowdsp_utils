#include <TimedUnitTest.h>

class IRHelpersTest : public TimedUnitTest
{
public:
    IRHelpersTest() : TimedUnitTest ("IRHelpers Test") {}

    void linearPhaseTest()
    {
        constexpr int pow2Factor = 4;
        constexpr int irLength = 1 << pow2Factor;

        dsp::FFT fft { pow2Factor };
        float irData[irLength] {};
        irData[0] = 1.0f;
        irData[1] = 0.8f;
        irData[2] = 0.6f;
        irData[3] = 0.4f;
        irData[4] = 0.2f;

        chowdsp::IRHelpers::makeLinearPhase (irData, irData, irLength, fft);

        static constexpr float idealIR[] = { 0.00122966f, -0.04306514f, -0.08621752f, -0.12724954f, -0.26536237f, -0.37576117f, -0.46107902f, -0.52022055f, -0.54182007f, 0.67842844f, 0.569897f, 0.46625725f, 0.35244531f, 0.22483731f, 0.08317718f, 0.04450323f };
        for (int i = 0; i < irLength; ++i)
            expectWithinAbsoluteError (irData[i], idealIR[i], 1.0e-6f, "Linear phase IR value is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Linear Phase Test");
        linearPhaseTest();
    }
};

static IRHelpersTest irHelpersTest;
