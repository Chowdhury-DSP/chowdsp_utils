#include <iostream>
#include <wdf.h>

using namespace chowdsp::WDF;

/**
 * Since the WDF library should be able to be used
 * for non-JUCE things, this standalone test is to
 * make sure that the WDF code builds correctly and
 * can be correctly used without JUCE.
 */
int main()
{
    std::cout << "Set up WDF Tests:" << std::endl;

    Resistor<float> R1 { 1.0e3f };
    ResistiveVoltageSource<float> Vin { 1.0e3f };
    PolarityInverter<float> I1 { &Vin };
    WDFSeries<float> S1 { &R1, &I1 };
    DiodePair<float> D1 { &S1, 1.0e-10f };

    float inVoltage = 10.0f;
    std::cout << "Setting input voltage: " << inVoltage << std::endl;

    Vin.setVoltage (inVoltage);
    D1.incident (S1.reflected());
    S1.incident (D1.reflected());
    auto outVoltage = R1.voltage();

    std::cout << "Output voltage: " << outVoltage << std::endl;

    if (std::abs (outVoltage - 4.77f) > 0.1f)
    {
        std::cout << "Incorrect output voltage!" << std::endl;
        return 1;
    }

    std::cout << "Correct output voltage!" << std::endl;
    return 0;
}
