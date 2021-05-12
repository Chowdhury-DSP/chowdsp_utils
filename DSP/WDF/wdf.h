#ifndef WDF_H_INCLUDED
#define WDF_H_INCLUDED

#include "omega.h"
#include <cmath>
#include <string>

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
    /** Wave digital filter base class */
    template <typename T>
    class WDF
    {
    public:
        WDF (std::string type) : type (type) {}
        virtual ~WDF() {}

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

        template <typename, typename Port1Type, typename Port2Type>
        friend class WDFParallelT;

        template <typename, typename Port1Type, typename Port2Type>
        friend class WDFSeriesT;

        T R = (T) 1.0e-9; // impedance
        T G = (T) 1.0 / R; // admittance

    protected:
        using FloatType = T;
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
        virtual ~WDFNode() {}

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
            WDF<T>::calcImpedance();

            if (next != nullptr)
                next->propagateImpedance();
        }

    protected:
        WDF<T>* next = nullptr;
    };

    /** WDF Resistor Node */
    template <typename T>
    class Resistor : public WDFNode<T>
    {
    public:
        /** Creates a new WDF Resistor with a given resistance.
     * @param value: resistance in Ohms
     */
        Resistor (T value) : WDFNode<T> ("Resistor"),
                             R_value (value)
        {
            calcImpedance();
        }
        virtual ~Resistor() {}

        /** Sets the resistance value of the WDF resistor, in Ohms. */
        void setResistanceValue (T newR)
        {
            if (newR == R_value)
                return;

            R_value = newR;
            WDFNode<T>::propagateImpedance();
        }

        /** Computes the impedance of the WDF resistor, Z_R = R. */
        inline void calcImpedance() override
        {
            this->R = R_value;
            this->G = (T) 1.0 / this->R;
        }

        /** Accepts an incident wave into a WDF resistor. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF resistor. */
        inline T reflected() noexcept override
        {
            this->b = 0.0;
            return this->b;
        }

    private:
        T R_value = (T) 1.0e-9;
    };

    /** WDF Capacitor Node */
    template <typename T>
    class Capacitor : public WDFNode<T>
    {
    public:
        /** Creates a new WDF Capacitor.
     * @param value: Capacitance value in Farads
     * @param fs: WDF sample rate
     * @param alpha: alpha value to be used for the alpha transform,
     *               use 0 for Backwards Euler, use 1 for Bilinear Transform.
     */
        Capacitor (T value, T fs, T alpha = 1.0) : WDFNode<T> ("Capacitor"),
                                                   C_value (value),
                                                   fs (fs),
                                                   alpha (alpha),
                                                   b_coef (((T) 1.0 - alpha) / (T) 2.0),
                                                   a_coef (((T) 1.0 + alpha) / (T) 2.0)
        {
            calcImpedance();
        }
        virtual ~Capacitor() {}

        /** Sets the capacitance value of the WDF capacitor, in Farads. */
        void setCapacitanceValue (T newC)
        {
            if (newC == C_value)
                return;

            C_value = newC;
            WDFNode<T>::propagateImpedance();
        }

        /** Computes the impedance of the WDF capacitor,
     *                 1
     * Z_C = ---------------------
     *       (1 + alpha) * f_s * C
     */
        inline void calcImpedance() override
        {
            this->R = (T) 1.0 / (((T) 1.0 + alpha) * C_value * fs);
            this->G = (T) 1.0 / this->R;
        }

        /** Accepts an incident wave into a WDF capacitor. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
            z = this->a;
        }

        /** Propogates a reflected wave from a WDF capacitor. */
        inline T reflected() noexcept override
        {
            this->b = b_coef * this->b + a_coef * z;
            return this->b;
        }

    private:
        T C_value = (T) 1.0e-6;
        T z = (T) 0.0;

        const T fs;
        const T alpha;

        const T b_coef;
        const T a_coef;
    };

    /** WDF Inductor Node */
    template <typename T>
    class Inductor : public WDFNode<T>
    {
    public:
        /** Creates a new WDF Inductor.
     * @param value: Inductance value in Farads
     * @param fs: WDF sample rate
     * @param alpha: alpha value to be used for the alpha transform,
     *               use 0 for Backwards Euler, use 1 for Bilinear Transform.
     */
        Inductor (T value, T fs, T alpha = 1.0) : WDFNode<T> ("Inductor"),
                                                  L_value (value),
                                                  fs (fs),
                                                  alpha (alpha),
                                                  b_coef (((T) 1.0 - alpha) / (T) 2.0),
                                                  a_coef (((T) 1.0 + alpha) / (T) 2.0)
        {
            calcImpedance();
        }
        virtual ~Inductor() {}

        /** Sets the inductance value of the WDF capacitor, in Henries. */
        void setInductanceValue (T newL)
        {
            if (newL == L_value)
                return;

            L_value = newL;
            WDFNode<T>::propagateImpedance();
        }

        /** Computes the impedance of the WDF capacitor,
     * Z_L = (1 + alpha) * f_s * L
     */
        inline void calcImpedance() override
        {
            this->R = ((T) 1.0 + alpha) * L_value * fs;
            this->G = (T) 1.0 / this->R;
        }

        /** Accepts an incident wave into a WDF inductor. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
            z = this->a;
        }

        /** Propogates a reflected wave from a WDF inductor. */
        inline T reflected() noexcept override
        {
            this->b = b_coef * this->b - a_coef * z;
            return this->b;
        }

    private:
        T L_value = (T) 1.0e-6;
        T z = (T) 0.0;

        const T fs;
        const T alpha;

        const T b_coef;
        const T a_coef;
    };

    /** WDF Switch (non-adaptable) */
    template <typename T>
    class Switch : public WDFNode<T>
    {
    public:
        Switch() : WDFNode<T> ("Switch")
        {
        }
        virtual ~Switch() {}

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
    class Open : public WDFNode<T>
    {
    public:
        Open() : WDFNode<T> ("Open")
        {
        }
        virtual ~Open()
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
    class Short : public WDFNode<T>
    {
    public:
        Short() : WDFNode<T> ("Short")
        {
        }
        virtual ~Short()
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
    class PolarityInverter : public WDFNode<T>
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
        virtual ~PolarityInverter() {}

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
            port1->incident (-x);
        }

        /** Propogates a reflected wave from a WDF inverter. */
        inline T reflected() noexcept override
        {
            this->b = -port1->reflected();
            return this->b;
        }

    private:
        WDFNode<T>* port1 = nullptr;
    };

    /** WDF y-parameter 2-port (short circuit admittance) */
    template <typename T>
    class YParameter : public WDFNode<T>
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

        virtual ~YParameter() {}

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
        virtual ~WDFAdaptor() {}

    protected:
        WDFNode<T>* port1 = nullptr;
        WDFNode<T>* port2 = nullptr;
    };

    /** WDF 3-port parallel adaptor */
    template <typename T>
    class WDFParallel : public WDFAdaptor<T>
    {
    public:
        /** Creates a new WDF parallel adaptor from two connected ports. */
        WDFParallel (WDFNode<T>* port1, WDFNode<T>* port2) : WDFAdaptor<T> (port1, port2, "Parallel")
        {
            calcImpedance();
        }
        virtual ~WDFParallel() {}

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
            this->port1->incident (x + (this->port2->b - this->port1->b) * port2Reflect);
            this->port2->incident (x + (this->port2->b - this->port1->b) * -port1Reflect);
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF parallel adaptor. */
        inline T reflected() noexcept override
        {
            this->b = port1Reflect * this->port1->reflected() + port2Reflect * this->port2->reflected();
            return this->b;
        }

    private:
        T port1Reflect = (T) 1.0;
        T port2Reflect = (T) 1.0;
    };

    /** WDF 3-port series adaptor */
    template <typename T>
    class WDFSeries : public WDFAdaptor<T>
    {
    public:
        /** Creates a new WDF series adaptor from two connected ports. */
        WDFSeries (WDFNode<T>* port1, WDFNode<T>* port2) : WDFAdaptor<T> (port1, port2, "Series")
        {
            calcImpedance();
        }
        virtual ~WDFSeries() {}

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
            this->port1->incident (this->port1->b - port1Reflect * (x + this->port1->b + this->port2->b));
            this->port2->incident (this->port2->b - port2Reflect * (x + this->port1->b + this->port2->b));
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF series adaptor. */
        inline T reflected() noexcept override
        {
            this->b = -(this->port1->reflected() + this->port2->reflected());
            return this->b;
        }

    private:
        T port1Reflect = (T) 1.0;
        T port2Reflect = (T) 1.0;
    };

    /** WDF Voltage source with series resistance */
    template <typename T>
    class ResistiveVoltageSource : public WDFNode<T>
    {
    public:
        /** Creates a new resistive voltage source.
     * @param value: initial resistance value, in Ohms
     */
        ResistiveVoltageSource (T value = 1.0e-9) : WDFNode<T> ("Resistive Voltage"),
                                                    R_value (value)
        {
            calcImpedance();
        }
        virtual ~ResistiveVoltageSource() {}

        /** Sets the resistance value of the series resistor, in Ohms. */
        void setResistanceValue (T newR)
        {
            if (newR == R_value)
                return;

            R_value = newR;
            WDFNode<T>::propagateImpedance();
        }

        /** Computes the impedance for a WDF resistive voltage souce
     * Z_Vr = Z_R
     */
        inline void calcImpedance() override
        {
            this->R = R_value;
            this->G = (T) 1.0 / this->R;
        }

        /** Sets the voltage of the voltage source, in Volts */
        void setVoltage (T newV) { Vs = newV; }

        /** Accepts an incident wave into a WDF resistive voltage source. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF resistive voltage source. */
        inline T reflected() noexcept override
        {
            this->b = Vs;
            return this->b;
        }

    private:
        T Vs = (T) 0.0;
        T R_value = (T) 1.0e-9;
    };

    /** WDF Ideal Voltage source (non-adaptable) */
    template <typename T>
    class IdealVoltageSource : public WDFNode<T>
    {
    public:
        IdealVoltageSource() : WDFNode<T> ("IdealVoltage")
        {
            calcImpedance();
        }
        virtual ~IdealVoltageSource() {}

        inline void calcImpedance() override {}

        /** Sets the voltage of the voltage source, in Volts */
        void setVoltage (T newV) { Vs = newV; }

        /** Accepts an incident wave into a WDF ideal voltage source. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF ideal voltage source. */
        inline T reflected() noexcept override
        {
            this->b = -this->a + (T) 2.0 * Vs;
            return this->b;
        }

    private:
        T Vs = (T) 0.0;
    };

    /** WDF Current source with parallel resistance */
    template <typename T>
    class ResistiveCurrentSource : public WDFNode<T>
    {
    public:
        /** Creates a new resistive current source.
     * @param value: initial resistance value, in Ohms
     */
        ResistiveCurrentSource (T value = 1.0e9) : WDFNode<T> ("Resistive Current"),
                                                   R_value (value)
        {
            calcImpedance();
        }
        virtual ~ResistiveCurrentSource() {}

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
    class IdealCurrentSource : public WDFNode<T>
    {
    public:
        IdealCurrentSource() : WDFNode<T> ("Ideal Current")
        {
            calcImpedance();
        }
        virtual ~IdealCurrentSource() {}

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

    /** Signum function to determine the sign of the input. */
    template <typename T>
    inline int signum (T val)
    {
        return (T (0) < val) - (val < T (0));
    }

    /** WDF diode pair (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
    template <typename T>
    class DiodePair : public WDFNode<T>
    {
    public:
        /** Creates a new WDF diode pair, with the given diode specifications.
     * @param Is: reverse saturation current
     * @param Vt: thermal voltage
     */
        DiodePair (T Is, T Vt) : WDFNode<T> ("DiodePair"),
                                 Is (Is),
                                 Vt (Vt)
        {
        }

        virtual ~DiodePair() {}

        inline void calcImpedance() override {}

        /** Accepts an incident wave into a WDF diode pair. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF diode pair. */
        inline T reflected() noexcept override
        {
            // See eqn (18) from reference paper
            T lambda = (T) signum (this->a);
            this->b = this->a + (T) 2 * lambda * (this->next->R * Is - Vt * Omega::omega4 (std::log (this->next->R * Is / Vt) + (lambda * this->a + this->next->R * Is) / Vt));
            return this->a;
        }

    private:
        const T Is; // reverse saturation current
        const T Vt; // thermal voltage
    };

    /** WDF diode (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
    template <typename T>
    class Diode : public WDFNode<T>
    {
    public:
        /** Creates a new WDF diode, with the given diode specifications.
     * @param Is: reverse saturation current
     * @param Vt: thermal voltage
     */
        Diode (T Is, T Vt) : WDFNode<T> ("Diode"),
                             Is (Is),
                             Vt (Vt)
        {
        }

        virtual ~Diode() {}

        inline void calcImpedance() override {}

        /** Accepts an incident wave into a WDF diode. */
        inline void incident (T x) noexcept override
        {
            this->a = x;
        }

        /** Propogates a reflected wave from a WDF diode. */
        inline T reflected() noexcept override
        {
            // See eqn (10) from reference paper
            this->b = this->a + (T) 2 * this->next->R * Is - 2 * Vt * Omega::omega4 (std::log (this->next->R * Is / Vt) + (this->a + this->next->R * Is) / Vt);
            return this->b;
        }

    private:
        const T Is; // reverse saturation current
        const T Vt; // thermal voltage
    };

} // namespace WDF

} // namespace chowdsp

#endif // WDF_H_INCLUDED
