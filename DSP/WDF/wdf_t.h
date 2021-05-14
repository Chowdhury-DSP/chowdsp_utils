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
    template <typename T, typename Port1Type, typename Port2Type>
    class WDFParallelT : public WDFNode<T>
    {
    public:
        /** Creates a new WDF parallel adaptor from two connected ports. */
        WDFParallelT (Port1Type& p1, Port2Type& p2) : chowdsp::WDF::WDFNode<T> ("Parallel"),
                                                      port1 (p1),
                                                      port2 (p2)
        {
            port1.connectToNode (this);
            port2.connectToNode (this);
            calcImpedance();
        }

        /** Computes the impedance for a WDF parallel adaptor.
     *  1     1     1
     * --- = --- + ---
     * Z_p   Z_1   Z_2
     */
        inline void calcImpedance() override
        {
            this->G = port1.G + port2.G;
            this->R = 1.0 / this->G;
            port1Reflect = port1.G / this->G;
            port2Reflect = port2.G / this->G;
        }

        /** Accepts an incident wave into a WDF parallel adaptor. */
        inline void incident (T x) noexcept override
        {
            port1.incident (x + (port2.b - port1.b) * port2Reflect);
            port2.incident (x + (port2.b - port1.b) * ((T) 0 - port1Reflect));
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF parallel adaptor. */
        inline T reflected() noexcept override
        {
            this->b = port1Reflect * port1.reflected() + port2Reflect * port2.reflected();
            return this->b;
        }

        Port1Type& port1;
        Port2Type& port2;

    private:
        T port1Reflect = (T) 1.0;
        T port2Reflect = (T) 1.0;
    };

    /** WDF 3-port series adaptor */
    template <typename T, typename Port1Type, typename Port2Type>
    class WDFSeriesT : public WDFNode<T>
    {
    public:
        /** Creates a new WDF series adaptor from two connected ports. */
        WDFSeriesT (Port1Type& p1, Port2Type& p2) : WDFNode<T> ("Series"),
                                                    port1 (p1),
                                                    port2 (p2)
        {
            port1.connectToNode (this);
            port2.connectToNode (this);
            calcImpedance();
        }

        /** Computes the impedance for a WDF parallel adaptor.
     * Z_s = Z_1 + Z_2
     */
        inline void calcImpedance() override
        {
            this->R = port1.R + port2.R;
            this->G = 1.0 / this->R;
            port1Reflect = port1.R / this->R;
            port2Reflect = port2.R / this->R;
        }

        /** Accepts an incident wave into a WDF series adaptor. */
        inline void incident (T x) noexcept override
        {
            port1.incident (port1.b - port1Reflect * (x + port1.b + port2.b));
            port2.incident (port2.b - port2Reflect * (x + port1.b + port2.b));

            this->a = x;
        }

        /** Propogates a reflected wave from a WDF series adaptor. */
        inline T reflected() noexcept override
        {
            this->b = (T) 0 - (port1.reflected() + port2.reflected());
            return this->b;
        }

        Port1Type& port1;
        Port2Type& port2;

    private:
        T port1Reflect = (T) 1.0;
        T port2Reflect = (T) 1.0;
    };

    /** WDF Voltage Polarity Inverter */
    template <typename T, typename PortType>
    class PolarityInverterT : public WDFNode<T>
    {
    public:
        /** Creates a new WDF polarity inverter */
        PolarityInverterT (PortType& p) : WDFNode<T> ("Polarity Inverter"),
                                          port1 (p)
        {
            port1.connectToNode (this);
            calcImpedance();
        }

        /** Calculates the impedance of the WDF inverter
     * (same impedance as the connected node).
     */
        inline void calcImpedance() override
        {
            this->R = port1.R;
            this->G = 1.0 / this->R;
        }

        /** Accepts an incident wave into a WDF inverter. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
            port1.incident ((T) 0 - x);
        }

        /** Propogates a reflected wave from a WDF inverter. */
        inline T reflected() noexcept override
        {
            this->b = (T) 0 - port1.reflected();
            return this->b;
        }

    private:
        PortType& port1;
    };

    // useful "factory" functions to you don't have to declare all the template parameters

    template <typename T, typename P1Type, typename P2Type>
    WDFParallelT<T, P1Type, P2Type> makeParallel (P1Type& p1, P2Type& p2)
    {
        return WDFParallelT<T, P1Type, P2Type> (p1, p2);
    }

    template <typename T, typename P1Type, typename P2Type>
    WDFSeriesT<T, P1Type, P2Type> makeSeries (P1Type& p1, P2Type& p2)
    {
        return WDFSeriesT<T, P1Type, P2Type> (p1, p2);
    }

    template <typename T, typename PType>
    PolarityInverterT<T, PType> makeInverter (PType& p1)
    {
        return PolarityInverterT<T, PType> (p1);
    }

} // namespace WDF
} // namespace chowdsp

#endif // WDF_T_INCLUDED
