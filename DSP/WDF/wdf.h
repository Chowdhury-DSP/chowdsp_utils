#ifndef WDF_H_INCLUDED
#define WDF_H_INCLUDED

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
    class WDF : public WDFT::BaseWDF
    {
        using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

    public:
        WDF (std::string type) : type (type) {}
        virtual ~WDF() = default;

        void connectToNode (WDF<T>* p) { parent = p; }

        /** Sub-classes override this function to propagate
     * an impedance change to the upstream elements in
     * the WDF tree.
     */
        virtual inline void propagateImpedance()
        {
            calcImpedance();

            if (parent != nullptr)
                parent->propagateImpedance();
        }

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

        T a = (T) 0.0; // incident wave
        T b = (T) 0.0; // reflected wave

    private:
        const std::string type;

        WDF<T>* parent = nullptr;
    };

    template <typename T, typename WDFType>
    class WDFWrapper : public WDF<T>
    {
    public:
        template <typename... Args>
        WDFWrapper (std::string name, Args&&... args) : WDF<T> (name),
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

    template <typename T, typename WDFType>
    class WDFRootWrapper : public WDFWrapper<T, WDFType>
    {
    public:
        template <typename Next, typename... Args>
        WDFRootWrapper (std::string name, Next& next, Args&&... args) : WDFWrapper<T, WDFType> (name, std::forward<Args> (args)...)
        {
            next.connectToNode (this);
            calcImpedance();
        }

        inline void propagateImpedance() override
        {
            this->calcImpedance();
        }

        inline void calcImpedance() override
        {
            this->internalWDF.calcImpedance();
        }
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
            this->propagateImpedance();
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
            this->propagateImpedance();
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
            this->propagateImpedance();
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
            this->propagateImpedance();
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
            this->propagateImpedance();
        }
    };

    /** WDF Switch (non-adaptable) */
    template <typename T>
    class Switch final : public WDF<T>
    {
    public:
        Switch() : WDF<T> ("Switch")
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
    class Open final : public WDF<T>
    {
    public:
        Open() : WDF<T> ("Open")
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
    class Short final : public WDF<T>
    {
    public:
        Short() : WDF<T> ("Short")
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
    class PolarityInverter final : public WDFWrapper<T, WDFT::PolarityInverterT<T, WDF<T>>>
    {
    public:
        /** Creates a new WDF polarity inverter
         * @param port1: the port to connect to the inverter
         */
        PolarityInverter (WDF<T>* port1) : WDFWrapper<T, WDFT::PolarityInverterT<T, WDF<T>>> ("Polarity Inverter", *port1)
        {
            port1->connectToNode (this);
        }
    };

    /** WDF y-parameter 2-port (short circuit admittance) */
    template <typename T>
    class YParameter final : public WDF<T>
    {
    public:
        YParameter (WDF<T>* port1, T y11, T y12, T y21, T y22) : WDF<T> ("YParameter"),
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
        WDF<T>* port1;
        T y[2][2] = { { (T) 0.0, (T) 0.0 }, { (T) 0.0, (T) 0.0 } };

        T denominator = (T) 1.0;
        T A = (T) 1.0;
        T B = (T) 1.0;
        T C = (T) 1.0;
    };

    /** WDF 3-port parallel adaptor */
    template <typename T>
    class WDFParallel final : public WDFWrapper<T, WDFT::WDFParallelT<T, WDF<T>, WDF<T>>>
    {
    public:
        /** Creates a new WDF parallel adaptor from two connected ports. */
        WDFParallel (WDF<T>* port1, WDF<T>* port2) : WDFWrapper<T, WDFT::WDFParallelT<T, WDF<T>, WDF<T>>> ("Parallel", *port1, *port2)
        {
            port1->connectToNode (this);
            port2->connectToNode (this);
        }
    };

    /** WDF 3-port series adaptor */
    template <typename T>
    class WDFSeries final : public WDFWrapper<T, WDFT::WDFSeriesT<T, WDF<T>, WDF<T>>>
    {
    public:
        /** Creates a new WDF series adaptor from two connected ports. */
        WDFSeries (WDF<T>* port1, WDF<T>* port2) : WDFWrapper<T, WDFT::WDFSeriesT<T, WDF<T>, WDF<T>>> ("Series", *port1, *port2)
        {
            port1->connectToNode (this);
            port2->connectToNode (this);
        }
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
            this->propagateImpedance();
        }

        /** Sets the voltage of the voltage source, in Volts */
        void setVoltage (T newV) { this->internalWDF.setVoltage (newV); }
    };

    /** WDF Ideal Voltage source (non-adaptable) */
    template <typename T>
    class IdealVoltageSource final : public WDFWrapper<T, WDFT::IdealVoltageSourceT<T, WDF<T>>>
    {
    public:
        IdealVoltageSource (WDF<T>* next) : WDFWrapper<T, WDFT::IdealVoltageSourceT<T, WDF<T>>> ("IdealVoltage", *next)
        {
            next->connectToNode (this);
        }

        /** Sets the voltage of the voltage source, in Volts */
        void setVoltage (T newV) { this->internalWDF.setVoltage (newV); }
    };

    /** WDF Current source with parallel resistance */
    template <typename T>
    class ResistiveCurrentSource final : public WDFWrapper<T, WDFT::ResistiveCurrentSourceT<T>>
    {
        using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

    public:
        /** Creates a new resistive current source.
     * @param value: initial resistance value, in Ohms
     */
        ResistiveCurrentSource (T value = (NumericType) 1.0e9) : WDFWrapper<T, WDFT::ResistiveCurrentSourceT<T>> ("Resistive Current", value)
        {
        }

        /** Sets the resistance value of the parallel resistor, in Ohms. */
        void setResistanceValue (T newR)
        {
            this->internalWDF.setResistanceValue (newR);
            this->propagateImpedance();
        }

        /** Sets the current of the current source, in Amps */
        void setCurrent (T newI) { this->internalWDF.setCurrent (newI); }
    };

    /** WDF Current source (non-adpatable) */
    template <typename T>
    class IdealCurrentSource final : public WDFWrapper<T, WDFT::IdealCurrentSourceT<T, WDF<T>>>
    {
    public:
        IdealCurrentSource (WDF<T>* next) : WDFWrapper<T, WDFT::IdealCurrentSourceT<T, WDF<T>>> ("Ideal Current", *next)
        {
            next->connectToNode (this);
        }

        /** Sets the current of the current source, in Amps */
        void setCurrent (T newI) { this->internalWDF.setCurrent (newI); }
    };

    /** WDF diode pair (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
    template <typename T>
    class DiodePair final : public WDFRootWrapper<T, WDFT::DiodePairT<T, WDF<T>>>
    {
    public:
        /** Creates a new WDF diode pair, with the given diode specifications.
         * @param Is: reverse saturation current
         * @param Vt: thermal voltage
         */
        DiodePair (T Is, T Vt, WDF<T>* next) : WDFRootWrapper<T, WDFT::DiodePairT<T, WDF<T>>> ("DiodePair", *next, Is, Vt, *next)
        {
            next->connectToNode (this);
        }
    };

    /** WDF diode (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
    template <typename T>
    class Diode final : public WDFRootWrapper<T, WDFT::DiodeT<T, WDF<T>>>
    {
    public:
        /** Creates a new WDF diode, with the given diode specifications.
     * @param Is: reverse saturation current
     * @param Vt: thermal voltage
     */
        Diode (T Is, T Vt, WDF<T>* next) : WDFRootWrapper<T, WDFT::DiodeT<T, WDF<T>>> ("Diode", *next, Is, Vt, *next)
        {
            next->connectToNode (this);
        }
    };

} // namespace WDF
} // namespace chowdsp

#undef USING_JUCE

#endif // WDF_H_INCLUDED
