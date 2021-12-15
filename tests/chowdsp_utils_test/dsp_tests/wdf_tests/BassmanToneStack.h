#pragma once

#include <JuceHeader.h>

using namespace chowdsp;

/** Fender Bassman tonestack circuit */
class Tonestack
{
public:
    Tonestack() = default;

    void prepare (double sampleRate)
    {
        Cap1.prepare (sampleRate);
        Cap2.prepare (sampleRate);
        Cap3.prepare (sampleRate);
    }

    double processSample (double inSamp)
    {
        Vres.setVoltage (inSamp);
        R.compute();

        return WDFT::voltage<double> (Res1m) + WDFT::voltage<double> (S2) + WDFT::voltage<double> (Res3m);
    }

    void setParams (double highPot, double lowPot, double midPot)
    {
        Res1m.setResistanceValue (highPot * R1);
        Res1p.setResistanceValue ((1.0 - highPot) * R1);

        Res2.setResistanceValue ((1.0 - lowPot) * R2);

        Res3m.setResistanceValue (midPot * R3);
        Res3p.setResistanceValue ((1.0 - midPot) * R3);
    }

private:
    WDFT::CapacitorAlphaT<double> Cap1 { 250e-12 };
    WDFT::CapacitorAlphaT<double> Cap2 { 20e-9 }; // Port D
    WDFT::CapacitorAlphaT<double> Cap3 { 20e-9 }; // Port F

    WDFT::ResistorT<double> Res1p { 1.0 };
    WDFT::ResistorT<double> Res1m { 1.0 };
    WDFT::ResistorT<double> Res2 { 1.0 };
    WDFT::ResistorT<double> Res3p { 1.0 };
    WDFT::ResistorT<double> Res3m { 1.0 };
    WDFT::ResistorT<double> Res4 { 56e3 }; // Port E

    WDFT::ResistiveVoltageSourceT<double> Vres { 1.0 };

    // Port A
    WDFT::WDFSeriesT<double, decltype (Vres), decltype (Res3m)> S1 { Vres, Res3m };

    // Port B
    WDFT::WDFSeriesT<double, decltype (Res2), decltype (Res3p)> S3 { Res2, Res3p };

    // Port C
    WDFT::WDFSeriesT<double, decltype (Res1p), decltype (Res1m)> S4 { Res1p, Res1m };
    WDFT::WDFSeriesT<double, decltype (Cap1), decltype (S4)> S2 { Cap1, S4 };

    static constexpr double R1 = 250e3;
    static constexpr double R2 = 1e6;
    static constexpr double R3 = 25e3;

    struct ImpedanceCalc
    {
        template <typename RType>
        static void calcImpedance (RType& R)
        {
            const auto [Ra, Rb, Rc, Rd, Re, Rf] = R.getPortImpedances();
            const auto Ga = 1.0 / Ra;
            const auto Gb = 1.0 / Rb;
            const auto Gc = 1.0 / Rc;
            const auto Gd = 1.0 / Rd;
            const auto Ge = 1.0 / Re;
            const auto Gf = 1.0 / Rf;

            // This scattering matrix was derived using the R-Solver python script (https://github.com/jatinchowdhury18/R-Solver),
            // with netlist input: netlists/bassman.txt
            R.setSMatrixData ({ { 2 * Ra * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Ga * Gb * Gd * Ge - Ga * Gb * Ge * Gf - Ga * Gc * Gd * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Ra * (Ga * Gb * Gc * Gd + Ga * Gb * Gc * Ge + Ga * Gb * Gc * Gf + Ga * Gb * Gd * Ge) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Ra * (Ga * Gb * Gc * Gd + Ga * Gb * Gc * Ge + Ga * Gb * Gc * Gf + Ga * Gc * Gd * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Ra * (Ga * Gb * Gd * Ge - Ga * Gc * Gd * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Ra * (-Ga * Gb * Gd * Ge - Ga * Gb * Ge * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Ra * (-Ga * Gb * Ge * Gf - Ga * Gc * Gd * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Rb * (Ga * Gb * Gc * Gd + Ga * Gb * Gc * Ge + Ga * Gb * Gc * Gf + Ga * Gb * Gd * Ge) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rb * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Ga * Gb * Gd * Ge - Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gc * Ge * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Rb * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rb * (-Ga * Gb * Gd * Ge - Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rb * (Ga * Gb * Gd * Ge - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rb * (-Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gc * Ge * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Rc * (Ga * Gb * Gc * Gd + Ga * Gb * Gc * Ge + Ga * Gb * Gc * Gf + Ga * Gc * Gd * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rc * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rc * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Ga * Gc * Gd * Ge - Ga * Gc * Gd * Gf - Gb * Gc * Gd * Ge - Gb * Gc * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Rc * (Ga * Gc * Gd * Ge + Ga * Gc * Gd * Gf + Gb * Gc * Gd * Ge + Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rc * (-Ga * Gc * Gd * Ge - Gb * Gc * Gd * Ge - Gb * Gc * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rc * (Ga * Gc * Gd * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Rd * (Ga * Gb * Gd * Ge - Ga * Gc * Gd * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rd * (-Ga * Gb * Gd * Ge - Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rd * (Ga * Gc * Gd * Ge + Ga * Gc * Gd * Gf + Gb * Gc * Gd * Ge + Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rd * (-Ga * Gb * Gd * Ge - Ga * Gb * Gd * Gf - Ga * Gc * Gd * Ge - Ga * Gc * Gd * Gf - Gb * Gc * Gd * Ge - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Rd * (Ga * Gb * Gd * Ge + Ga * Gc * Gd * Ge + Gb * Gc * Gd * Ge + Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rd * (-Ga * Gb * Gd * Gf - Ga * Gc * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Re * (-Ga * Gb * Gd * Ge - Ga * Gb * Ge * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Re * (Ga * Gb * Gd * Ge - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Re * (-Ga * Gc * Gd * Ge - Gb * Gc * Gd * Ge - Gb * Gc * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Re * (Ga * Gb * Gd * Ge + Ga * Gc * Gd * Ge + Gb * Gc * Gd * Ge + Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Re * (-Ga * Gb * Gd * Ge - Ga * Gb * Ge * Gf - Ga * Gc * Gd * Ge - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf - Gb * Gc * Gd * Ge - Gb * Gc * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Re * (-Ga * Gb * Ge * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Rf * (-Ga * Gb * Ge * Gf - Ga * Gc * Gd * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (-Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gc * Ge * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (Ga * Gc * Gd * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (-Ga * Gb * Gd * Gf - Ga * Gc * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (-Ga * Gb * Ge * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (-Ga * Gb * Gd * Gf - Ga * Gb * Ge * Gf - Ga * Gc * Gd * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf - Gb * Gc * Gd * Gf - Gb * Gc * Ge * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1 } });
        }
    };

    using RType = WDFT::RootRtypeAdaptor<double, ImpedanceCalc, decltype (S1), decltype (S3), decltype (S2), decltype (Cap2), decltype (Res4), decltype (Cap3)>;
    RType R { std::tie (S1, S3, S2, Cap2, Res4, Cap3) };
};
