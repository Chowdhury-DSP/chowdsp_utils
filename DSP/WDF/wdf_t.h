#ifndef WDF_T_INCLUDED
#define WDF_T_INCLUDED

namespace chowdsp
{
/**
 * Templated adaptors for Wave Digital Filters. These are faster than
 * the polymorphic adaptors, but less flexible!
 */
namespace WDF
{
    /** WDF 3-port parallel adaptor */
    template <typename Port1Type, typename Port2Type>
    class WDFParallelT : public WDFNode
    {
    public:
        /** Creates a new WDF parallel adaptor from two connected ports. */
        WDFParallelT (Port1Type& p1, Port2Type& p2) : chowdsp::WDF::WDFNode ("Parallel"),
                                                      port1 (p1),
                                                      port2 (p2)
        {
            port1.connectToNode (this);
            port2.connectToNode (this);
            calcImpedance();
        }
        virtual ~WDFParallelT() {}

        /** Computes the impedance for a WDF parallel adaptor.
     *  1     1     1
     * --- = --- + ---
     * Z_p   Z_1   Z_2
     */
        inline void calcImpedance() override
        {
            G = port1.G + port2.G;
            R = 1.0 / G;
            port1Reflect = port1.G / G;
            port2Reflect = port2.G / G;
        }

        /** Accepts an incident wave into a WDF parallel adaptor. */
        inline void incident (double x) noexcept override
        {
            port1.incident (x + (port2.b - port1.b) * port2Reflect);
            port2.incident (x + (port2.b - port1.b) * -port1Reflect);
            a = x;
        }

        /** Propogates a reflected wave from a WDF parallel adaptor. */
        inline double reflected() noexcept override
        {
            b = port1Reflect * port1.reflected() + port2Reflect * port2.reflected();
            return b;
        }

        Port1Type& port1;
        Port2Type& port2;

    private:
        double port1Reflect = 1.0;
        double port2Reflect = 1.0;
    };

    /** WDF 3-port series adaptor */
    template <typename Port1Type, typename Port2Type>
    class WDFSeriesT : public WDFNode
    {
    public:
        /** Creates a new WDF series adaptor from two connected ports. */
        WDFSeriesT (Port1Type& p1, Port2Type& p2) : WDFNode ("Series"),
                                                    port1 (p1),
                                                    port2 (p2)
        {
            port1.connectToNode (this);
            port2.connectToNode (this);
            calcImpedance();
        }
        virtual ~WDFSeriesT() {}

        /** Computes the impedance for a WDF parallel adaptor.
     * Z_s = Z_1 + Z_2
     */
        inline void calcImpedance() override
        {
            R = port1.R + port2.R;
            G = 1.0 / R;
            port1Reflect = port1.R / R;
            port2Reflect = port2.R / R;
        }

        /** Accepts an incident wave into a WDF series adaptor. */
        inline void incident (double x) noexcept override
        {
            port1.incident (port1.b - port1Reflect * (x + port1.b + port2.b));
            port2.incident (port2.b - port2Reflect * (x + port1.b + port2.b));

            a = x;
        }

        /** Propogates a reflected wave from a WDF series adaptor. */
        inline double reflected() noexcept override
        {
            b = -(port1.reflected() + port2.reflected());
            return b;
        }

        Port1Type& port1;
        Port2Type& port2;

    private:
        double port1Reflect = 1.0;
        double port2Reflect = 1.0;
    };

    /** WDF Voltage Polarity Inverter */
    template <typename PortType>
    class PolarityInverterT : public WDFNode
    {
    public:
        /** Creates a new WDF polarity inverter */
        PolarityInverterT (PortType& p) : WDFNode ("Polarity Inverter"),
                                          port1 (p)
        {
            port1.connectToNode (this);
            calcImpedance();
        }
        virtual ~PolarityInverterT() {}

        /** Calculates the impedance of the WDF inverter
     * (same impedance as the connected node).
     */
        inline void calcImpedance() override
        {
            R = port1.R;
            G = 1.0 / R;
        }

        /** Accepts an incident wave into a WDF inverter. */
        inline void incident (double x) noexcept override
        {
            a = x;
            port1.incident (-x);
        }

        /** Propogates a reflected wave from a WDF inverter. */
        inline double reflected() noexcept override
        {
            b = -port1.reflected();
            return b;
        }

    private:
        PortType& port1;
    };

} // namespace WDF
} // namespace chowdsp

#endif // WDF_T_INCLUDED
