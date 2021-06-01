#ifndef WDF_H_INCLUDED
#define WDF_H_INCLUDED

#include <cmath>
#include <string>
#include "wdf_t.h"

// we want to be able to use this header without JUCE, so let's #if out JUCE-specific implementations
#define USING_JUCE JUCE_WINDOWS || JUCE_ANDROID || JUCE_BSD || JUCE_LINUX || JUCE_MAC || JUCE_IOS || JUCE_WASM
#include "signum.h"
#include "omega.h"

namespace chowdsp
{
/**
 * A framework for creating circuit emulations with Wave Digital Filters.
 * For more technical information, see:
 * - https://www.eit.lth.se/fileadmin/eit/courses/eit085f/Fettweis_Wave_Digital_Filters_Theory_and_Practice_IEEE_Proc_1986_-_This_is_a_real_challange.pdf
 * - https://searchworks.stanford.edu/view/11891203
 * 
 * To start, initialize all your circuit elements and connections.
 * Be sure to pick a "root" node, and call `root.connectToNode (adaptor);`
 * 
 * To run the simulation, call the following code
 * once per sample:
 * ```
 * // set source inputs here...
 * 
 * root.incident (adaptor.reflected());
 * // if probing the root node, do that here...
 * adaptor.incident (root.reflected());
 * 
 * // probe other elements here...
 * ```
 * 
 * To probe a node, call `element.voltage()` or `element.current()`.
 */
namespace WDF
{
#if USING_JUCE
    using namespace SIMDUtils;
#endif // USING_JUCE

    /** Wave digital filter base class */
    template <typename T>
    class WDF
    {
        using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

    public:
        WDF (std::string type) : type (type) {}
        virtual ~WDF() = default;

        /** Sub-classes override this function to recompute
     * the impedance of this element.
     */
        virtual void calcImpedance() {}

        /** Sub-classes override this function to propogate
     * an impedance change to the upstream elements in
     * the WDF tree.
     */
        virtual void propagateImpedance() = 0;

        /** Sub-classes override this function to accept an incident wave. */
        virtual void incident (T x) noexcept = 0;

        /** Sub-classes override this function to propogate a reflected wave. */
        virtual T reflected() noexcept = 0;

        /** Probe the voltage across this circuit element. */
        inline T voltage() const noexcept
        {
            return (a + b) / (T) 2.0;
        }

        /**Probe the current through this circuit element. */
        inline T current() const noexcept
        {
            return (a - b) / ((T) 2.0 * R);
        }

        // These classes need access to a,b
        template <typename>
        friend class YParameter;

        template <typename>
        friend class WDFParallel;

        template <typename>
        friend class WDFSeries;

        T R = (NumericType) 1.0e-9; // impedance
        T G = (T) 1.0 / R; // admittance

    protected:
        T a = (T) 0.0; // incident wave
        T b = (T) 0.0; // reflected wave

    private:
        const std::string type;
    };

    /** WDF node base class */
    template <typename T>
    class WDFNode : public WDF<T>
    {
    public:
        WDFNode (std::string type) : WDF<T> (type) {}

        /** Connects this WDF node to an upstream node in the WDF tree. */
        void connectToNode (WDF<T>* node)
        {
            next = node;
        }

        /** When this function is called from a downstream
     * element in the WDF tree, the impedance is recomputed
     * and then propogated upstream to the next element in the
     * WDF tree.
     */
        inline void propagateImpedance() override
        {
            this->calcImpedance();

            if (next != nullptr)
                next->propagateImpedance();
        }

    protected:
        WDF<T>* next = nullptr;
    };

    template <typename T, typename WDFType>
    class WDFWrapper : public WDFNode<T>
    {
    public:
        template <typename... Args>
        WDFWrapper (std::string name, Args&&... args) : WDFNode<T> (name),
                                                        internalWDF (std::forward<Args> (args)...)
        {
            calcImpedance();
        }

        /** Computes the impedance of the WDF resistor, Z_R = R. */
        inline void calcImpedance() override
        {
            internalWDF.calcImpedance();
            this->R = internalWDF.R;
            this->G = internalWDF.G;
        }

        /** Accepts an incident wave into a WDF resistor. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
            internalWDF.incident (x);
        }

        /** Propogates a reflected wave from a WDF resistor. */
        inline T reflected() noexcept override
        {
            this->b = internalWDF.reflected();
            return this->b;
        }

    protected:
        WDFType internalWDF;
    };

    /** WDF Resistor Node */
    template <typename T>
    class Resistor final : public WDFWrapper<T, WDFT::ResistorT<T>>
    {
    public:
        /** Creates a new WDF Resistor with a given resistance.
         * @param value: resistance in Ohms
         */
        Resistor (T value) : WDFWrapper<T, WDFT::ResistorT<T>> ("Resistor", value)
        {
        }

        /** Sets the resistance value of the WDF resistor, in Ohms. */
        void setResistanceValue (T newR)
        {
            this->internalWDF.setResistanceValue (newR);
            WDFNode<T>::propagateImpedance();
        }
    };

    /** WDF Capacitor Node */
    template <typename T>
    class Capacitor final : public WDFWrapper<T, WDFT::CapacitorT<T>>
    {
    public:
        /** Creates a new WDF Capacitor.
         * @param value: Capacitance value in Farads
         * @param fs: WDF sample rate
         */
        Capacitor (T value, T fs) : WDFWrapper<T, WDFT::CapacitorT<T>> ("Capacitor", value, fs)
        {
        }

        /** Sets the capacitance value of the WDF capacitor, in Farads. */
        void setCapacitanceValue (T newC)
        {
            this->internalWDF.setCapacitanceValue (newC);
            WDFNode<T>::propagateImpedance();
        }
    };

    /** WDF Capacitor Node with alpha transform parameter */
    template <typename T>
    class CapacitorAlpha final : public WDFWrapper<T, WDFT::CapacitorAlphaT<T>>
    {
    public:
        /** Creates a new WDF Capacitor.
         * @param value: Capacitance value in Farads
         * @param fs: WDF sample rate
         * @param alpha: alpha value to be used for the alpha transform,
         *               use 0 for Backwards Euler, use 1 for Bilinear Transform.
         */
        CapacitorAlpha (T value, T fs, T alpha = 1.0) : WDFWrapper<T, WDFT::CapacitorAlphaT<T>> ("Capacitor", value, fs, alpha)
        {
        }

        /** Sets the capacitance value of the WDF capacitor, in Farads. */
        void setCapacitanceValue (T newC)
        {
            this->internalWDF.setCapacitanceValue (newC);
            WDFNode<T>::propagateImpedance();
        }
    };

    /** WDF Inductor Node */
    template <typename T>
    class Inductor final : public WDFWrapper<T, WDFT::InductorT<T>>
    {
    public:
        /** Creates a new WDF Inductor.
     * @param value: Inductance value in Farads
     * @param fs: WDF sample rate
     */
        Inductor (T value, T fs) : WDFWrapper<T, WDFT::InductorT<T>> ("Inductor", value, fs)
        {
        }

        /** Sets the inductance value of the WDF inductor, in Henries. */
        void setInductanceValue (T newL)
        {
            this->internalWDF.setInductanceValue (newL);
            WDFNode<T>::propagateImpedance();
        }
    };

    /** WDF Inductor Node with alpha transform parameter */
    template <typename T>
    class InductorAlpha final : public WDFWrapper<T, WDFT::InductorAlphaT<T>>
    {
    public:
        /** Creates a new WDF Inductor.
     * @param value: Inductance value in Farads
     * @param fs: WDF sample rate
     * @param alpha: alpha value to be used for the alpha transform,
     *               use 0 for Backwards Euler, use 1 for Bilinear Transform.
     */
        InductorAlpha (T value, T fs, T alpha = 1.0) : WDFWrapper<T, WDFT::InductorAlphaT<T>> ("Inductor", value, fs, alpha)
        {
        }

        /** Sets the inductance value of the WDF inductor, in Henries. */
        void setInductanceValue (T newL)
        {
            this->internalWDF.setInductanceValue (newL);
            WDFNode<T>::propagateImpedance();
        }
    };

    /** WDF Switch (non-adaptable) */
    template <typename T>
    class Switch final : public WDFNode<T>
    {
    public:
        Switch() : WDFNode<T> ("Switch")
        {
        }

        inline void calcImpedance() override {}

        /** Sets the state of the switch. */
        void setClosed (bool shouldClose) { closed = shouldClose; }

        /** Accepts an incident wave into a WDF switch. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF switch. */
        inline T reflected() noexcept override
        {
            this->b = closed ? -this->a : this->a;
            return this->b;
        }

    private:
        bool closed = true;
    };

    /** WDF Open circuit (non-adaptable) */
    template <typename T>
    class Open final : public WDFNode<T>
    {
    public:
        Open() : WDFNode<T> ("Open")
        {
            this->R = (T) 1.0e15;
            this->G = (T) 1.0 / this->R;
        }

        inline void calcImpedance() override {}

        /** Accepts an incident wave into a WDF open. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF open. */
        inline T reflected() noexcept override
        {
            this->b = this->a;
            return this->b;
        }
    };

    /** WDF Short circuit (non-adaptable) */
    template <typename T>
    class Short final : public WDFNode<T>
    {
    public:
        Short() : WDFNode<T> ("Short")
        {
            this->R = (T) 1.0e-15;
            this->G = (T) 1.0 / this->R;
        }

        inline void calcImpedance() override {}

        /** Accepts an incident wave into a WDF short. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF short. */
        inline T reflected() noexcept override
        {
            this->b = -this->a;
            return this->b;
        }
    };

    /** WDF Voltage Polarity Inverter */
    template <typename T>
    class PolarityInverter final : public WDFNode<T>
    {
    public:
        /** Creates a new WDF polarity inverter
         * @param port1: the port to connect to the inverter
         */
        PolarityInverter (WDFNode<T>* port1) : WDFNode<T> ("Polarity Inverter"),
                                               port1 (port1)
        {
            port1->connectToNode (this);
            calcImpedance();
        }

        /** Calculates the impedance of the WDF inverter
     * (same impedance as the connected node).
     */
        inline void calcImpedance() override
        {
            this->R = port1->R;
            this->G = (T) 1.0 / this->R;
        }

        /** Accepts an incident wave into a WDF inverter. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
            port1->incident ((T) 0 - x);
        }

        /** Propogates a reflected wave from a WDF inverter. */
        inline T reflected() noexcept override
        {
            this->b = (T) 0 - port1->reflected();
            return this->b;
        }

    private:
        WDFNode<T>* port1 = nullptr;
    };

    /** WDF y-parameter 2-port (short circuit admittance) */
    template <typename T>
    class YParameter final : public WDFNode<T>
    {
    public:
        YParameter (WDFNode<T>* port1, T y11, T y12, T y21, T y22) : WDFNode<T> ("YParameter"),
                                                                     port1 (port1)
        {
            y[0][0] = y11;
            y[0][1] = y12;
            y[1][0] = y21;
            y[1][1] = y22;

            port1->connectToNode (this);
            calcImpedance();
        }

        inline void calcImpedance() override
        {
            denominator = y[1][1] + port1->R * y[0][0] * y[1][1] - port1->R * y[0][1] * y[1][0];
            this->R = (port1->R * y[0][0] + (T) 1.0) / denominator;
            this->G = (T) 1.0 / this->R;

            T rSq = port1->R * port1->R;
            T num1A = -y[1][1] * rSq * y[0][0] * y[0][0];
            T num2A = y[0][1] * y[1][0] * rSq * y[0][0];

            A = (num1A + num2A + y[1][1]) / (denominator * (port1->R * y[0][0] + (T) 1.0));
            B = -port1->R * y[0][1] / (port1->R * y[0][0] + (T) 1.0);
            C = -y[1][0] / denominator;
        }

        inline void incident (T x) noexcept override
        {
            this->a = x;
            port1->incident (A * port1->b + B * x);
        }

        inline T reflected() noexcept override
        {
            this->b = C * port1->reflected();
            return this->b;
        }

    private:
        WDFNode<T>* port1;
        T y[2][2] = { { (T) 0.0, (T) 0.0 }, { (T) 0.0, (T) 0.0 } };

        T denominator = (T) 1.0;
        T A = (T) 1.0;
        T B = (T) 1.0;
        T C = (T) 1.0;
    };

    /** WDF 3-port adapter base class */
    template <typename T>
    class WDFAdaptor : public WDFNode<T>
    {
    public:
        WDFAdaptor (WDFNode<T>* port1, WDFNode<T>* port2, std::string type) : WDFNode<T> (type),
                                                                              port1 (port1),
                                                                              port2 (port2)
        {
            port1->connectToNode (this);
            port2->connectToNode (this);
        }

    protected:
        WDFNode<T>* port1 = nullptr;
        WDFNode<T>* port2 = nullptr;
    };

    /** WDF 3-port parallel adaptor */
    template <typename T>
    class WDFParallel final : public WDFAdaptor<T>
    {
    public:
        /** Creates a new WDF parallel adaptor from two connected ports. */
        WDFParallel (WDFNode<T>* port1, WDFNode<T>* port2) : WDFAdaptor<T> (port1, port2, "Parallel")
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
            this->G = this->port1->G + this->port2->G;
            this->R = (T) 1.0 / this->G;
            port1Reflect = this->port1->G / this->G;
            port2Reflect = this->port2->G / this->G;
        }

        /** Accepts an incident wave into a WDF parallel adaptor. */
        inline void incident (T x) noexcept override
        {
            auto b2 = x + bTemp;
            this->port1->incident (bDiff + b2);
            this->port2->incident (b2);
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF parallel adaptor. */
        inline T reflected() noexcept override
        {
            this->port1->reflected();
            this->port2->reflected();

            bDiff = this->port2->b - this->port1->b;
            bTemp = (T) 0 - port1Reflect * bDiff;
            this->b = this->port2->b + bTemp;

            return this->b;
        }

    private:
        T port1Reflect = (T) 1.0;
        T port2Reflect = (T) 1.0;

        T bTemp = (T) 0.0;
        T bDiff = (T) 0.0;
    };

    /** WDF 3-port series adaptor */
    template <typename T>
    class WDFSeries final : public WDFAdaptor<T>
    {
    public:
        /** Creates a new WDF series adaptor from two connected ports. */
        WDFSeries (WDFNode<T>* port1, WDFNode<T>* port2) : WDFAdaptor<T> (port1, port2, "Series")
        {
            calcImpedance();
        }

        /** Computes the impedance for a WDF parallel adaptor.
     * Z_s = Z_1 + Z_2
     */
        inline void calcImpedance() override
        {
            this->R = this->port1->R + this->port2->R;
            this->G = (T) 1.0 / this->R;
            port1Reflect = this->port1->R / this->R;
            port2Reflect = this->port2->R / this->R;
        }

        /** Accepts an incident wave into a WDF series adaptor. */
        inline void incident (T x) noexcept override
        {
            auto b1 = this->port1->b - port1Reflect * (x + this->port1->b + this->port2->b);
            this->port1->incident (b1);
            this->port2->incident ((T) 0 - (x + b1));
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF series adaptor. */
        inline T reflected() noexcept override
        {
            this->b = (T) 0 - (this->port1->reflected() + this->port2->reflected());
            return this->b;
        }

    private:
        T port1Reflect = (T) 1.0;
        T port2Reflect = (T) 1.0;
    };

    /** WDF Voltage source with series resistance */
    template <typename T>
    class ResistiveVoltageSource final : public WDFWrapper<T, WDFT::ResistiveVoltageSourceT<T>>
    {
        using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

    public:
        /** Creates a new resistive voltage source.
     * @param value: initial resistance value, in Ohms
     */
        ResistiveVoltageSource (T value = (NumericType) 1.0e-9) : WDFWrapper<T, WDFT::ResistiveVoltageSourceT<T>> ("Resistive Voltage", value)
        {
        }

        /** Sets the resistance value of the series resistor, in Ohms. */
        void setResistanceValue (T newR)
        {
            this->internalWDF.setResistanceValue (newR);
            WDFNode<T>::propagateImpedance();
        }

        /** Sets the voltage of the voltage source, in Volts */
        void setVoltage (T newV) { this->internalWDF.setVoltage (newV); }
    };

    /** WDF Ideal Voltage source (non-adaptable) */
    template <typename T>
    class IdealVoltageSource final : public WDFWrapper<T, WDFT::IdealVoltageSourceT<T>>
    {
    public:
        IdealVoltageSource() : WDFWrapper<T, WDFT::IdealVoltageSourceT<T>> ("IdealVoltage")
        {
        }

        inline void calcImpedance() override {}

        /** Sets the voltage of the voltage source, in Volts */
        void setVoltage (T newV) { this->internalWDF.setVoltage (newV); }
    };

    /** WDF Current source with parallel resistance */
    template <typename T>
    class ResistiveCurrentSource final : public WDFNode<T>
    {
        using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

    public:
        /** Creates a new resistive current source.
     * @param value: initial resistance value, in Ohms
     */
        ResistiveCurrentSource (T value = (NumericType) 1.0e9) : WDFNode<T> ("Resistive Current"),
                                                                 R_value (value)
        {
            calcImpedance();
        }

        /** Sets the resistance value of the parallel resistor, in Ohms. */
        void setResistanceValue (T newR)
        {
            if (newR == R_value)
                return;

            R_value = newR;
            WDFNode<T>::propagateImpedance();
        }

        /** Computes the impedance for a WDF resistive current souce
     * Z_Ir = Z_R
     */
        inline void calcImpedance() override
        {
            this->R = R_value;
            this->G = (T) 1.0 / this->R;
        }

        /** Sets the current of the current source, in Amps */
        void setCurrent (T newI) { Is = newI; }

        /** Accepts an incident wave into a WDF resistive current source. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF resistive current source. */
        inline T reflected() noexcept override
        {
            this->b = (T) 2.0 * this->R * Is;
            return this->b;
        }

    private:
        T Is = (T) 0.0;
        T R_value = (T) 1.0e9;
    };

    /** WDF Current source (non-adpatable) */
    template <typename T>
    class IdealCurrentSource final : public WDFNode<T>
    {
    public:
        IdealCurrentSource() : WDFNode<T> ("Ideal Current")
        {
            calcImpedance();
        }

        inline void calcImpedance() override {}

        /** Sets the current of the current source, in Amps */
        void setCurrent (T newI) { Is = newI; }

        /** Accepts an incident wave into a WDF ideal current source. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF ideal current source. */
        inline T reflected() noexcept override
        {
            this->b = (T) 2.0 * this->next->R * Is + this->a;
            return this->b;
        }

    private:
        T Is = (T) 0.0;
    };

    /** WDF diode pair (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
    template <typename T>
    class DiodePair final : public WDFWrapper<T, WDFT::DiodePairT<T, WDFNode<T>>>
    {
    public:
        /** Creates a new WDF diode pair, with the given diode specifications.
         * @param Is: reverse saturation current
         * @param Vt: thermal voltage
         */
        DiodePair (T Is, T Vt, WDFNode<T>* next) : WDFWrapper<T, WDFT::DiodePairT<T, WDFNode<T>>> ("DiodePair", Is, Vt, *next)
        {
        }

        inline void calcImpedance() override {}
    };

    /** WDF diode (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
    template <typename T>
    class Diode final : public WDFWrapper<T, WDFT::DiodeT<T, WDFNode<T>>>
    {
    public:
        /** Creates a new WDF diode, with the given diode specifications.
     * @param Is: reverse saturation current
     * @param Vt: thermal voltage
     */
        Diode (T Is, T Vt, WDFNode<T>* next) : WDFWrapper<T, WDFT::DiodeT<T, WDFNode<T>>> ("Diode", Is, Vt, *next)
        {
        }

        inline void calcImpedance() override {}
    };

} // namespace WDF

} // namespace chowdsp

#undef USING_JUCE

#endif // WDF_H_INCLUDED
