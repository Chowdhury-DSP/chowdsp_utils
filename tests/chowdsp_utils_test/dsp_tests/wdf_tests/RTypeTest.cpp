#include <test_utils.h>
#include <TimedUnitTest.h>

namespace
{
constexpr double _fs = 44100.0;
} // namespace

using namespace chowdsp;

/** Fender Bassman tonestack circuit */
class Tonestack
{
public:
    Tonestack (double sr) : Cap1 (250e-12, sr, alpha),
                            Cap2 (20e-9, sr, alpha),
                            Cap3 (20e-9, sr, alpha),
                            R (std::tie (S1, S3, S2, Cap2, Res4, Cap3))
    {
    }

    void setSMatrixData()
    {
        double Ra = S1.R;
        double Rb = S3.R;
        R.setSMatrixData ({
            { 1 - (2 * Ra * (348190.833333 * Rb + 362427505.6386863889)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), (421032094.8051527778 * Ra) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), (2 * Ra * (560000000000000.0 * Rb + 586045987915763889.0)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), -(2 * Ra * (56000 * Rb - 151911448.611)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(2 * Ra * (292190.833333 * Rb + 303822906.84711)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(2 * Ra * (348190.833333 * Rb + 151911458.23611)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463) },
            { (421032094.8051527778 * Rb) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), 1 - (2 * Rb * (348190.833333 * Ra + 16544036047.4025763889)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(2 * Rb * (520.833333 * Ra + 58604598.7915763889)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(2 * Rb * (347670.0 * Ra + 16485431448.611)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(2 * Rb * (520.833333 * Ra - 151911448.611)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(2 * Rb * (348190.833333 * Ra + 16333520000.0)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463) },
            { (326670400000000000000.0 * Rb + 341864066590781707009260.0) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), -(583340.0 * (520.833333 * Ra + 58604598.7915763889)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(1.0 * (2452183080555336111.0 * Ra + 164268268498194236111.0 * Rb + 2351491666670000.0 * Ra * Rb + 170932033295390853504630.0)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), (583340.0 * (520.8333 * Ra + 56520.8333 * Rb + Ra * Rb)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(583340.0 * (1041.666633 * Ra + 520.8333 * Rb + Ra * Rb)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(1.0 * (3038229164722200000.0 * Ra - 326670400000000000000.0 * Rb)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0) },
            { -(1.0 * (583333332960000000.0 * Rb - 1582410922018507009260.0)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), -(1041.666666 * (347670.0 * Ra + 16485431448.611)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), (1041.666666 * (520.8333 * Ra + 56520.8333 * Rb + Ra * Rb)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(1.0 * (2712788166863889.0 * Ra - 164851601794194236111.0 * Rb - 3471491666670000.0 * Ra * Rb + 791210851070853504630.0)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), (1041.666666 * (520.8333 * Ra + 520.8333 * Rb + Ra * Rb + 151911448.611)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(1.0 * (3621562497682200000.0 * Ra + 583333332960000000.0 * Rb + 170140833224443200000000.0)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0) },
            { -(1.0 * (327253733332960000000.0 * Rb + 340281655668763200000000.0)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), -(1.0 * (583333332960000000.0 * Ra - 170140822444320000000000.0)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), -(1.0 * (1166666628960000000.0 * Ra + 583333296000000000.0 * Rb + 1120000000000000.0 * Ra * Rb)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), (583333296000000000.0 * Ra + 583333296000000000.0 * Rb + 1120000000000000.0 * Ra * Rb + 170140822444320000000000.0) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), (2457608427426863889.0 * Ra - 162396706154934236111.0 * Rb + 2361908333330000.0 * Ra * Rb - 169349622373372346495370.0) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), -(1.0 * (583333332960000000.0 * Ra + 327253733332960000000.0 * Rb + 170140833224443200000000.0)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0) },
            { -(1041.6666 * (348190.833333 * Rb + 151911458.23611)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(1041.6666 * (348190.833333 * Ra + 16333520000.0)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), -(1.0 * (5425346871527778.0 * Ra - 583333296000000000.0 * Rb)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), -(1.0 * (3621562268220000000.0 * Ra + 583333296000000000.0 * Rb + 170140822444320000000000.0)) / (3624275056386863889.0 * Ra + 165440360474025763889.0 * Rb + 3481908333330000.0 * Ra * Rb + 170932033295390853504630.0), -(1041.6666 * (520.833333 * Ra + 292190.833333 * Rb + 151911458.23611)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463), 1 - (1041.6666 * (348190.833333 * Ra + 348190.833333 * Rb + 16485431458.23611)) / (362427505.6386863889 * Ra + 16544036047.4025763889 * Rb + 348190.833333 * Ra * Rb + 17093203329539.085350463) },
        });
    }

    double processSample (double inSamp)
    {
        Vres.setVoltage (inSamp);
        R.incident (0.0);

        return WDFT::voltage<double> (Res1m) + WDFT::voltage<double> (S2) + WDFT::voltage<double> (Res3m);
    }

    void setParams (double highPot, double lowPot, double midPot)
    {
        Res1m.setResistanceValue (highPot * R1);
        Res1p.setResistanceValue ((1.0 - highPot) * R1);

        Res2.setResistanceValue ((1.0 - lowPot) * R2);

        Res3m.setResistanceValue (midPot * R3);
        Res3p.setResistanceValue ((1.0 - midPot) * R3);

        setSMatrixData();
    }

private:
    WDFT::CapacitorAlphaT<double> Cap1;
    WDFT::CapacitorAlphaT<double> Cap2; // Port D
    WDFT::CapacitorAlphaT<double> Cap3; // Port F

    WDFT::ResistorT<double> Res1p { 1.0 };
    WDFT::ResistorT<double> Res1m { 1.0 };
    WDFT::ResistorT<double> Res2 { 1.0 };
    WDFT::ResistorT<double> Res3p { 1.0 };
    WDFT::ResistorT<double> Res3m { 1.0 };
    WDFT::ResistorT<double> Res4 { 56e3 }; // Port E

    WDFT::ResistiveVoltageSourceT<double> Vres { 1.0 };

    // Port A
    using S1Type = WDFT::WDFSeriesT<double, WDFT::ResistiveVoltageSourceT<double>, WDFT::ResistorT<double>>;
    S1Type S1 { Vres, Res3m };

    // Port B
    using SeriesRes = WDFT::WDFSeriesT<double, WDFT::ResistorT<double>, WDFT::ResistorT<double>>;
    SeriesRes S3 { Res2, Res3p };

    // Port C
    SeriesRes S4 { Res1p, Res1m };
    using S2Type = WDFT::WDFSeriesT<double, WDFT::CapacitorAlphaT<double>, SeriesRes>;
    S2Type S2 { Cap1, S4 };

    static constexpr double alpha = 1.0;

    static constexpr double R1 = 250e3;
    static constexpr double R2 = 1e6;
    static constexpr double R3 = 25e3;

    WDFT::RootRtypeAdaptor<double, S1Type, SeriesRes, S2Type, WDFT::CapacitorAlphaT<double>, WDFT::ResistorT<double>, WDFT::CapacitorAlphaT<double>> R;
};

class RTypeTest : public TimedUnitTest
{
public:
    RTypeTest() : TimedUnitTest ("Wave Digital Filter R-Type Test", "Wave Digital Filters") {}

    void freqTest (float lowPot, float highPot, float sineFreq, float expGainDB, float maxErr)
    {
        Tonestack tonestack { _fs };
        tonestack.setParams ((double) highPot, (double) lowPot, 1.0);

        auto buffer = test_utils::makeSineWave (sineFreq, (float) _fs, 1.0f);
        auto* x = buffer.getWritePointer (0);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
            x[n] = (float) tonestack.processSample ((double) x[n]);

        auto actualGainDB = Decibels::gainToDecibels (buffer.getMagnitude (1000, buffer.getNumSamples() - 1000));
        expectWithinAbsoluteError (actualGainDB, expGainDB, maxErr);
    }

    void runTestTimed() override
    {
        beginTest ("Bass Test");
        freqTest (0.5f, 0.0f, 60.0f, -8.0f, 0.5f);

        beginTest ("Treble Test");
        freqTest (1.0f, 1.0f, 15000.0f, 5.0f, 0.5f);
    }
};

static RTypeTest rTypeTest;
