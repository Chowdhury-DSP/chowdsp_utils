#ifndef WDF_T2_INCLUDED
#define WDF_T2_INCLUDED

#include "wdf.h"

#define CREATE_WDFT_MEMBERS \
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type; \
    T R = (NumericType) 1.0e-9; /* impedance */ \
    T G = (T) 1.0 / R; /* admittance */ \
    T a = (T) 0.0; /* incident wave */ \
    T b = (T) 0.0; /* reflected wave */

namespace chowdsp
{

namespace WDFT
{
    class BaseWDF
    {
    public:
        void connectToParent (BaseWDF* p) { parent = p; }

        virtual void calcImpedance() = 0;

        void propagateImpedance()
        {
            calcImpedance();

            if (parent != nullptr)
                parent->propagateImpedance();
        }

    protected:
        BaseWDF* parent = nullptr;
    };

    /** WDF Resistor Node */
    template <typename T>
    class ResistorT final : public BaseWDF
    {
    public:
        /** Creates a new WDF Resistor with a given resistance.
         * @param value: resistance in Ohms
         */
        ResistorT (T value) : R_value (value)
        {
            calcImpedance();
        }

        /** Sets the resistance value of the WDF resistor, in Ohms. */
        void setResistanceValue (T newR)
        {
            if (newR == R_value)
                return;

            R_value = newR;
            propagateImpedance();
        }

        /** Computes the impedance of the WDF resistor, Z_R = R. */
        inline void calcImpedance() override
        {
            R = R_value;
            G = (T) 1.0 / R;
        }

        /** Accepts an incident wave into a WDF resistor. */
        inline void incident (T x) noexcept
        {
            a = x;
        }

        /** Propogates a reflected wave from a WDF resistor. */
        inline T reflected() noexcept
        {
            b = 0.0;
            return b;
        }

        CREATE_WDFT_MEMBERS

    private:
        T R_value = (T) 1.0e-9;
    };

    /** WDF Capacitor Node */
    template <typename T>
    class CapacitorT final : public BaseWDF
    {
    public:
        /** Creates a new WDF Capacitor.
     * @param value: Capacitance value in Farads
     * @param fs: WDF sample rate
     */
        CapacitorT (T value, T fs) : C_value (value),
                                     fs (fs)
        {
            calcImpedance();
        }

        /** Sets the capacitance value of the WDF capacitor, in Farads. */
        void setCapacitanceValue (T newC)
        {
            if (newC == C_value)
                return;

            C_value = newC;
            propagateImpedance();
        }

        /** Computes the impedance of the WDF capacitor,
         *             1
         * Z_C = --------------
         *        2 * f_s * C
         */
        inline void calcImpedance() override
        {
            R = (T) 1.0 / ((T) 2.0 * C_value * fs);
            G = (T) 1.0 / R;
        }

        /** Accepts an incident wave into a WDF capacitor. */
        inline void incident (T x) noexcept
        {
            a = x;
            z = a;
        }

        /** Propogates a reflected wave from a WDF capacitor. */
        inline T reflected() noexcept
        {
            b = z;
            return b;
        }

        CREATE_WDFT_MEMBERS

    private:
        T C_value = (T) 1.0e-6;
        T z = (T) 0.0;

        const T fs;
    };

    /** WDF 3-port parallel adaptor */
    template <typename T, typename Port1Type, typename Port2Type>
    class WDFParallelTT final : public BaseWDF
    {
    public:
        /** Creates a new WDF parallel adaptor from two connected ports. */
        WDFParallelTT (Port1Type& p1, Port2Type& p2) : port1 (p1),
                                                       port2 (p2)
        {
            calcImpedance();
        }

        /** Computes the impedance for a WDF parallel adaptor.
         *  1     1     1
         * --- = --- + ---
         * Z_p   Z_1   Z_2
         */
        inline void calcImpedance() override
        {
            G = port1.G + port2.G;
            R = (T) 1.0 / G;
            port1Reflect = port1.G / G;
            port2Reflect = port2.G / G;
        }

        /** Accepts an incident wave into a WDF parallel adaptor. */
        inline void incident (T x) noexcept
        {
            auto b2 = x + bTemp;
            port1.incident (bDiff + b2);
            port2.incident (b2);
            a = x;
        }

        /** Propogates a reflected wave from a WDF parallel adaptor. */
        inline T reflected() noexcept
        {
            port1.reflected();
            port2.reflected();

            bDiff = port2.b - port1.b;
            bTemp = (T) 0 - port1Reflect * bDiff;
            b = port2.b + bTemp;

            return b;
        }

        Port1Type& port1;
        Port2Type& port2;

        CREATE_WDFT_MEMBERS

    private:
        T port1Reflect = (T) 1.0;
        T port2Reflect = (T) 1.0;

        T bTemp = (T) 0.0;
        T bDiff = (T) 0.0;
    };

    /** WDF 3-port series adaptor */
    template <typename T, typename Port1Type, typename Port2Type>
    class WDFSeriesTT final : public BaseWDF
    {
    public:
        /** Creates a new WDF series adaptor from two connected ports. */
        WDFSeriesTT (Port1Type& p1, Port2Type& p2) : port1 (p1),
                                                    port2 (p2)
        {
            calcImpedance();
        }

        /** Computes the impedance for a WDF parallel adaptor.
         * Z_s = Z_1 + Z_2
         */
        inline void calcImpedance() override
        {
            R = port1.R + port2.R;
            G = (T) 1.0 / R;
            port1Reflect = port1.R / R;
            port2Reflect = port2.R / R;
        }

        /** Accepts an incident wave into a WDF series adaptor. */
        inline void incident (T x) noexcept
        {
            auto b1 = port1.b - port1Reflect * (x + port1.b + port2.b);
            port1.incident (b1);
            port2.incident ((T) 0 - (x + b1));

            a = x;
        }

        /** Propogates a reflected wave from a WDF series adaptor. */
        inline T reflected() noexcept
        {
            b = (T) 0 - (port1.reflected() + port2.reflected());
            return b;
        }

        Port1Type& port1;
        Port2Type& port2;

        CREATE_WDFT_MEMBERS

    private:
        T port1Reflect = (T) 1.0;
        T port2Reflect = (T) 1.0;
    };

    /** WDF Ideal Voltage source (non-adaptable) */
    template <typename T>
    class IdealVoltageSourceT
    {
    public:
        IdealVoltageSourceT() = default;

        /** Sets the voltage of the voltage source, in Volts */
        void setVoltage (T newV) { Vs = newV; }

        /** Accepts an incident wave into a WDF ideal voltage source. */
        inline void incident (T x) noexcept
        {
            a = x;
        }

        /** Propogates a reflected wave from a WDF ideal voltage source. */
        inline T reflected() noexcept
        {
            b = (T) 0 - a + (T) 2.0 * Vs;
            return b;
        }

        CREATE_WDFT_MEMBERS

    private:
        T Vs = (T) 0.0;
    };

    /** Probe the voltage across this circuit element. */
    template <typename T, typename WDFType>
    inline T voltage (const WDFType& wdf) noexcept
    {
        return (wdf.a + wdf.b) * (T) 0.5;
    }

    /**Probe the current through this circuit element. */
    template <typename T, typename WDFType>
    inline T current (const WDFType& wdf) noexcept
    {
        return (wdf.a - wdf.b) * ((T) 0.5 * wdf.G);
    }

} // namespace WDFT

} // namespace chowdsp

#undef CREATE_WDFT_MEMBERS

#endif // WDF_T_INCLUDED
