#include <iostream>
#include <wdf_t.h>

using namespace chowdsp::WDFT;

int main()
{
    std::cout << "Set up WDF Tests:" << std::endl;

    ResistorT<float> R1 { 1.0e3f };
    ResistorT<float> R2 { 1.0e3f };
    auto S1 = makeSeries<float> (R1, R2);
    auto I1 = makeInverter<float> (S1);
    IdealVoltageSourceT<float, decltype (I1)> Vin { I1 };

    float inVoltage = 10.0f;
    std::cout << "Setting input voltage: " << inVoltage << std::endl;

    Vin.setVoltage (inVoltage);
    Vin.incident (I1.reflected());
    I1.incident (Vin.reflected());
    auto outVoltage = voltage<float> (R1);

    std::cout << "Output voltage: " << outVoltage << std::endl;

    if (outVoltage != 5.0f)
    {
        std::cout << "Incorrect output voltage!" << std::endl;
        return 1;
    }

    std::cout << "Correct output voltage!" << std::endl;
    return 0;
}
