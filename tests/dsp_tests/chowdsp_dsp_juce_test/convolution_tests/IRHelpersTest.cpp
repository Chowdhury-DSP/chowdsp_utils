#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

TEST_CASE ("IRHelpers Test", "[dsp][convolution]")
{
    SECTION ("Linear Phase Test")
    {
        static constexpr int pow2Factor = 4;
        static constexpr int irLength = 1 << pow2Factor;

        juce::dsp::FFT fft { pow2Factor };
        float irData[irLength] {};
        irData[0] = 1.0f;
        irData[1] = 0.8f;
        irData[2] = 0.6f;
        irData[3] = 0.4f;
        irData[4] = 0.2f;

        chowdsp::IRHelpers::makeLinearPhase (irData, irData, irLength, fft);

        static constexpr float idealIR[] = { 0.00122966f, -0.04306514f, -0.08621752f, -0.12724954f, -0.26536237f, -0.37576117f, -0.46107902f, -0.52022055f, -0.54182007f, 0.67842844f, 0.569897f, 0.46625725f, 0.35244531f, 0.22483731f, 0.08317718f, 0.04450323f };
        for (int i = 0; i < irLength; ++i)
            REQUIRE_MESSAGE (irData[i] == Catch::Approx { idealIR[i] }.margin (1.0e-6f), "Linear phase IR value is incorrect!");
    }

    SECTION ("Minumum Phase Test")
    {
        constexpr int pow2Factor = 4;
        constexpr int irLength = 1 << pow2Factor;

        juce::dsp::FFT fft { pow2Factor };
        float irData[irLength] = { 0.0f, 0.16666667f, 0.33333333f, 0.5f, 0.66666667f, 0.83333333f, 1.0f, 1.0f, 0.85714286f, 0.71428571f, 0.57142857f, 0.42857143f, 0.28571429f, 0.14285714f, 0.0f, 0.0f };

        chowdsp::IRHelpers::makeMinimumPhase (irData, irData, irLength, fft);

        static constexpr float idealIR[] = { 0.13861501f, 0.32544317f, 0.52749507f, 0.66966621f, 0.79626566f, 0.92809213f, 0.9691116f, 0.8975361f, 0.75961539f, 0.60798622f, 0.46236642f, 0.31255422f, 0.163782f, 0.04407101f, -0.03153687f, -0.07106336f };
        for (int i = 0; i < irLength; ++i)
            REQUIRE_MESSAGE (irData[i] == Catch::Approx { idealIR[i] }.margin (1.0e-6f), "Linear phase IR value is incorrect!");
    }

    SECTION ("Half Magnitude Test")
    {
        constexpr int pow2Factor = 5;
        constexpr int irLength = 1 << pow2Factor;
        float irData[irLength] = { -0.00345003f, -0.00702325f, -0.01059443f, -0.01415994f, -0.01771303f, -0.02124279f, -0.02473305f, -0.02816142f, -0.03149816f, -0.03470527f, -0.03773546f, -0.0405312f, -0.04302388f, -0.04513294f, -0.04676518f, -0.04781416f, 0.95182201f, -0.04781459f, -0.04677881f, -0.0451853f, -0.04315329f, -0.0407887f, -0.03818478f, -0.03542277f, -0.03257265f, -0.02969396f, -0.02683661f, -0.02404175f, -0.02134262f, -0.01876531f, -0.0163296f, -0.01404972f };

        juce::dsp::FFT fft { pow2Factor };
        chowdsp::IRHelpers::makeHalfMagnitude (irData, irData, irLength, fft);

        constexpr float idealIR[irLength] = { -0.01279729f, -0.01280593f, -0.01492435f, -0.01703021f, -0.01911994f, -0.02118711f, -0.02322209f, -0.02521173f, -0.02713904f, -0.02898288f, -0.03071758f, -0.03231268f, -0.03373263f, -0.03493648f, -0.03587767f, -0.0365038f, 0.96322761f, -0.03662695f, -0.03609816f, -0.03526165f, -0.03418453f, -0.03292766f, -0.03154588f, -0.03008828f, -0.02859841f, -0.02711465f, -0.02567045f, -0.02429465f, -0.02301173f, -0.02184212f, -0.02080241f, -0.01990556f };
        for (int i = 0; i < irLength; ++i)
            REQUIRE_MESSAGE (irData[i] == Catch::Approx { idealIR[i] }.margin (1.0e-6f), "Half magnitude IR value is incorrect!");
    }
}
