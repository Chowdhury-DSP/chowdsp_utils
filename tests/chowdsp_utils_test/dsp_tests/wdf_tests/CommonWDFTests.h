#pragma once

#include <JuceHeader.h>

template <typename FloatType>
inline void voltageDividerTest (UnitTest& test)
{
    using namespace chowdsp::WDF;
    Resistor<FloatType> r1 ((FloatType) 10000.0);
    Resistor<FloatType> r2 ((FloatType) 10000.0);

    WDFSeries<FloatType> s1 (&r1, &r2);
    PolarityInverter<FloatType> p1 (&s1);
    IdealVoltageSource<FloatType> vs { &p1 };

    vs.setVoltage ((FloatType) 10.0f);
    vs.incident (p1.reflected());
    p1.incident (vs.reflected());

    auto vOut = r2.voltage();
    if (vOut != (FloatType) 5.0)
        test.expect (false, "Voltage divider: incorrect voltage!");
}

template <typename FloatType>
inline void currentDividerTest (UnitTest& test)
{
    using namespace chowdsp::WDF;
    Resistor<FloatType> r1 ((FloatType) 10000.0);
    Resistor<FloatType> r2 ((FloatType) 10000.0);

    WDFParallel<FloatType> p1 (&r1, &r2);
    IdealCurrentSource<FloatType> is (&p1);

    is.setCurrent ((FloatType) 1.0f);
    is.incident (p1.reflected());
    p1.incident (is.reflected());

    auto iOut = r2.current();
    if (iOut != (FloatType) 0.5)
        test.expect (false, "Current divider: incorrect current!");
}
