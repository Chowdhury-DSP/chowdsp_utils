#ifndef WDF_T_INCLUDED
#define WDF_T_INCLUDED

#include <cmath>
#include <type_traits>

// we want to be able to use this header without JUCE, so let's #if out JUCE-specific implementations
#ifndef WDF_USING_JUCE
#define WDF_USING_JUCE JUCE_WINDOWS || JUCE_ANDROID || JUCE_BSD || JUCE_LINUX || JUCE_MAC || JUCE_IOS || JUCE_WASM
#endif

#include "signum.h"
#include "omega.h"

/** API for constructing Wave Digital Filters with a fixed compile-time architecture */
namespace chowdsp::WDFT
{
#if WDF_USING_JUCE
using namespace SIMDUtils;
#endif // WDF_USING_JUCE

/** Base WDF class for propagating impedance changes between elements */
class BaseWDF
{
public:
    void connectToParent (BaseWDF* p) { parent = p; }

    virtual void calcImpedance() = 0;

    inline virtual void propagateImpedanceChange()
    {
        calcImpedance();

        if (parent != nullptr)
            parent->propagateImpedanceChange();
    }

protected:
    BaseWDF* parent = nullptr;
};

/** Base class for propagating impedance changes into root WDF elements */
class RootWDF : public BaseWDF
{
public:
    inline void propagateImpedanceChange() override { calcImpedance(); }

private:
    // don't try to connect root nodes!
    void connectToParent (BaseWDF*) {}
};

#ifndef DOXYGEN
/** Helper struct for common WDF member variables */
template <typename T>
struct WDFMembers
{
#if WDF_USING_JUCE
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;
#else
    using NumericType = T;
#endif
    T R = (NumericType) 1.0e-9; /* impedance */
    T G = (T) 1.0 / R; /* admittance */
    T a = (T) 0.0; /* incident wave */
    T b = (T) 0.0; /* reflected wave */
};
#endif // DOXYGEN

/** WDF Resistor Node */
template <typename T>
class ResistorT final : public BaseWDF
{
public:
    /** Creates a new WDF Resistor with a given resistance.
         * @param value: resistance in Ohms
         */
    explicit ResistorT (T value) : R_value (value)
    {
        calcImpedance();
    }

    /** Sets the resistance value of the WDF resistor, in Ohms. */
    void setResistanceValue (T newR)
    {
        if (newR == R_value)
            return;

        R_value = newR;
        propagateImpedanceChange();
    }

    /** Computes the impedance of the WDF resistor, Z_R = R. */
    inline void calcImpedance() override
    {
        wdf.R = R_value;
        wdf.G = (T) 1.0 / wdf.R;
    }

    /** Accepts an incident wave into a WDF resistor. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF resistor. */
    inline T reflected() noexcept
    {
        wdf.b = 0.0;
        return wdf.b;
    }

    WDFMembers<T> wdf;

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
    explicit CapacitorT (T value, T fs = (T) 48000.0) : C_value (value),
                                                        fs (fs)
    {
        calcImpedance();
    }

    /** Prepares the capacitor to operate at a new sample rate */
    void prepare (T sampleRate)
    {
        fs = sampleRate;
        propagateImpedanceChange();

        reset();
    }

    /** Resets the capacitor state */
    void reset()
    {
        z = (T) 0.0;
    }

    /** Sets the capacitance value of the WDF capacitor, in Farads. */
    void setCapacitanceValue (T newC)
    {
        if (newC == C_value)
            return;

        C_value = newC;
        propagateImpedanceChange();
    }

    /** Computes the impedance of the WDF capacitor,
         *             1
         * Z_C = --------------
         *        2 * f_s * C
         */
    inline void calcImpedance() override
    {
        wdf.R = (T) 1.0 / ((T) 2.0 * C_value * fs);
        wdf.G = (T) 1.0 / wdf.R;
    }

    /** Accepts an incident wave into a WDF capacitor. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
        z = wdf.a;
    }

    /** Propogates a reflected wave from a WDF capacitor. */
    inline T reflected() noexcept
    {
        wdf.b = z;
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    T C_value = (T) 1.0e-6;
    T z = (T) 0.0;

    T fs;
};

/** WDF Capacitor Node with alpha transform parameter */
template <typename T>
class CapacitorAlphaT final : public BaseWDF
{
public:
    /** Creates a new WDF Capacitor.
     * @param value: Capacitance value in Farads
     * @param fs: WDF sample rate
     * @param alpha: alpha value to be used for the alpha transform,
     *             use 0 for Backwards Euler, use 1 for Bilinear Transform.
     */
    explicit CapacitorAlphaT (T value, T fs = (T) 48000.0, T alpha = (T) 1.0) : C_value (value),
                                                                                fs (fs),
                                                                                alpha (alpha),
                                                                                b_coef (((T) 1.0 - alpha) / (T) 2.0),
                                                                                a_coef (((T) 1.0 + alpha) / (T) 2.0)
    {
        calcImpedance();
    }

    /** Prepares the capacitor to operate at a new sample rate */
    void prepare (T sampleRate)
    {
        fs = sampleRate;
        propagateImpedanceChange();

        reset();
    }

    /** Resets the capacitor state */
    void reset()
    {
        z = (T) 0.0;
    }

    /** Sets a new alpha value to use for the alpha transform */
    void setAlpha (T newAlpha)
    {
        alpha = newAlpha;
        b_coef = ((T) 1.0 - alpha) / (T) 2.0;
        a_coef = ((T) 1.0 + alpha) / (T) 2.0;

        propagateImpedanceChange();
    }

    /** Sets the capacitance value of the WDF capacitor, in Farads. */
    void setCapacitanceValue (T newC)
    {
        if (newC == C_value)
            return;

        C_value = newC;
        propagateImpedanceChange();
    }

    /** Computes the impedance of the WDF capacitor,
         *                 1
         * Z_C = ---------------------
         *       (1 + alpha) * f_s * C
         */
    inline void calcImpedance() override
    {
        wdf.R = (T) 1.0 / (((T) 1.0 + alpha) * C_value * fs);
        wdf.G = (T) 1.0 / wdf.R;
    }

    /** Accepts an incident wave into a WDF capacitor. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
        z = wdf.a;
    }

    /** Propogates a reflected wave from a WDF capacitor. */
    inline T reflected() noexcept
    {
        wdf.b = b_coef * wdf.b + a_coef * z;
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    T C_value = (T) 1.0e-6;
    T z = (T) 0.0;

    T fs;

    T alpha;
    T b_coef;
    T a_coef;
};

/** WDF Inductor Node */
template <typename T>
class InductorT final : public BaseWDF
{
public:
    /** Creates a new WDF Inductor.
         * @param value: Inductance value in Farads
         * @param fs: WDF sample rate
         */
    explicit InductorT (T value, T fs = (T) 48000.0) : L_value (value),
                                                       fs (fs)
    {
        calcImpedance();
    }

    /** Prepares the inductor to operate at a new sample rate */
    void prepare (T sampleRate)
    {
        fs = sampleRate;
        propagateImpedanceChange();

        reset();
    }

    /** Resets the inductor state */
    void reset()
    {
        z = (T) 0.0;
    }

    /** Sets the inductance value of the WDF inductor, in Henries. */
    void setInductanceValue (T newL)
    {
        if (newL == L_value)
            return;

        L_value = newL;
        propagateImpedanceChange();
    }

    /** Computes the impedance of the WDF inductor,
         * Z_L = 2 * f_s * L
         */
    inline void calcImpedance() override
    {
        wdf.R = (T) 2.0 * L_value * fs;
        wdf.G = (T) 1.0 / wdf.R;
    }

    /** Accepts an incident wave into a WDF inductor. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
        z = wdf.a;
    }

    /** Propogates a reflected wave from a WDF inductor. */
    inline T reflected() noexcept
    {
        wdf.b = -z;
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    T L_value = (T) 1.0e-6;
    T z = (T) 0.0;

    T fs;
};

/** WDF Inductor Node with alpha transform parameter */
template <typename T>
class InductorAlphaT final : public BaseWDF
{
public:
    /** Creates a new WDF Inductor.
         * @param value: Inductance value in Farads
         * @param fs: WDF sample rate
         * @param alpha: alpha value to be used for the alpha transform,
         *               use 0 for Backwards Euler, use 1 for Bilinear Transform.
         */
    explicit InductorAlphaT (T value, T fs = (T) 48000.0, T alpha = (T) 1.0) : L_value (value),
                                                                               fs (fs),
                                                                               alpha (alpha),
                                                                               b_coef (((T) 1.0 - alpha) / (T) 2.0),
                                                                               a_coef (((T) 1.0 + alpha) / (T) 2.0)
    {
        calcImpedance();
    }

    /** Prepares the inductor to operate at a new sample rate */
    void prepare (T sampleRate)
    {
        fs = sampleRate;
        propagateImpedanceChange();

        reset();
    }

    /** Resets the inductor state */
    void reset()
    {
        z = (T) 0.0;
    }

    /** Sets a new alpha value to use for the alpha transform */
    void setAlpha (T newAlpha)
    {
        alpha = newAlpha;
        b_coef = ((T) 1.0 - alpha) / (T) 2.0;
        a_coef = ((T) 1.0 + alpha) / (T) 2.0;

        propagateImpedanceChange();
    }

    /** Sets the inductance value of the WDF inductor, in Henries. */
    void setInductanceValue (T newL)
    {
        if (newL == L_value)
            return;

        L_value = newL;
        propagateImpedanceChange();
    }

    /** Computes the impedance of the WDF inductor,
         * Z_L = (1 + alpha) * f_s * L
         */
    inline void calcImpedance() override
    {
        wdf.R = ((T) 1.0 + alpha) * L_value * fs;
        wdf.G = (T) 1.0 / wdf.R;
    }

    /** Accepts an incident wave into a WDF inductor. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
        z = wdf.a;
    }

    /** Propogates a reflected wave from a WDF inductor. */
    inline T reflected() noexcept
    {
        wdf.b = b_coef * wdf.b - a_coef * z;
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    T L_value = (T) 1.0e-6;
    T z = (T) 0.0;

    T fs;

    T alpha;
    T b_coef;
    T a_coef;
};

/** WDF 3-port parallel adaptor */
template <typename T, typename Port1Type, typename Port2Type>
class WDFParallelT final : public BaseWDF
{
public:
    /** Creates a new WDF parallel adaptor from two connected ports. */
    WDFParallelT (Port1Type& p1, Port2Type& p2) : port1 (p1),
                                                  port2 (p2)
    {
        port1.connectToParent (this);
        port2.connectToParent (this);
        calcImpedance();
    }

    /** Computes the impedance for a WDF parallel adaptor.
         *  1     1     1
         * --- = --- + ---
         * Z_p   Z_1   Z_2
         */
    inline void calcImpedance() override
    {
        wdf.G = port1.wdf.G + port2.wdf.G;
        wdf.R = (T) 1.0 / wdf.G;
        port1Reflect = port1.wdf.G / wdf.G;
    }

    /** Accepts an incident wave into a WDF parallel adaptor. */
    inline void incident (T x) noexcept
    {
        auto b2 = x + bTemp;
        port1.incident (bDiff + b2);
        port2.incident (b2);
        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF parallel adaptor. */
    inline T reflected() noexcept
    {
        port1.reflected();
        port2.reflected();

        bDiff = port2.wdf.b - port1.wdf.b;
        bTemp = (T) 0 - port1Reflect * bDiff;
        wdf.b = port2.wdf.b + bTemp;

        return wdf.b;
    }

    Port1Type& port1;
    Port2Type& port2;

    WDFMembers<T> wdf;

private:
    T port1Reflect = (T) 1.0;

    T bTemp = (T) 0.0;
    T bDiff = (T) 0.0;
};

/** WDF 3-port series adaptor */
template <typename T, typename Port1Type, typename Port2Type>
class WDFSeriesT final : public BaseWDF
{
public:
    /** Creates a new WDF series adaptor from two connected ports. */
    WDFSeriesT (Port1Type& p1, Port2Type& p2) : port1 (p1),
                                                port2 (p2)
    {
        port1.connectToParent (this);
        port2.connectToParent (this);
        calcImpedance();
    }

    /** Computes the impedance for a WDF parallel adaptor.
         * Z_s = Z_1 + Z_2
         */
    inline void calcImpedance() override
    {
        wdf.R = port1.wdf.R + port2.wdf.R;
        wdf.G = (T) 1.0 / wdf.R;
        port1Reflect = port1.wdf.R / wdf.R;
    }

    /** Accepts an incident wave into a WDF series adaptor. */
    inline void incident (T x) noexcept
    {
        auto b1 = port1.wdf.b - port1Reflect * (x + port1.wdf.b + port2.wdf.b);
        port1.incident (b1);
        port2.incident ((T) 0 - (x + b1));

        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF series adaptor. */
    inline T reflected() noexcept
    {
        wdf.b = (T) 0 - (port1.reflected() + port2.reflected());
        return wdf.b;
    }

    Port1Type& port1;
    Port2Type& port2;

    WDFMembers<T> wdf;

private:
    T port1Reflect = (T) 1.0;
};

/** WDF Voltage Polarity Inverter */
template <typename T, typename PortType>
class PolarityInverterT final : public BaseWDF
{
public:
    /** Creates a new WDF polarity inverter */
    explicit PolarityInverterT (PortType& p) : port1 (p)
    {
        port1.connectToParent (this);
        calcImpedance();
    }

    /** Calculates the impedance of the WDF inverter
         * (same impedance as the connected node).
         */
    inline void calcImpedance() override
    {
        wdf.R = port1.wdf.R;
        wdf.G = (T) 1.0 / wdf.R;
    }

    /** Accepts an incident wave into a WDF inverter. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
        port1.incident ((T) 0 - x);
    }

    /** Propogates a reflected wave from a WDF inverter. */
    inline T reflected() noexcept
    {
        wdf.b = (T) 0 - port1.reflected();
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    PortType& port1;
};

/** WDF y-parameter 2-port (short circuit admittance) */
template <typename T, typename PortType>
class YParameterT final : public BaseWDF
{
public:
    /** Creates a new WDF Y-Parameter, with the given coefficients */
    YParameterT (PortType& port1, T y11, T y12, T y21, T y22) : port1 (port1)
    {
        y[0][0] = y11;
        y[0][1] = y12;
        y[1][0] = y21;
        y[1][1] = y22;

        port1.connectToParent (this);
        calcImpedance();
    }

    /** Calculates the impedance of the WDF Y-Parameter */
    inline void calcImpedance() override
    {
        denominator = y[1][1] + port1.wdf.R * y[0][0] * y[1][1] - port1.wdf.R * y[0][1] * y[1][0];
        wdf.R = (port1.wdf.R * y[0][0] + (T) 1.0) / denominator;
        wdf.G = (T) 1.0 / wdf.R;

        T rSq = port1.wdf.R * port1.wdf.R;
        T num1A = -y[1][1] * rSq * y[0][0] * y[0][0];
        T num2A = y[0][1] * y[1][0] * rSq * y[0][0];

        A = (num1A + num2A + y[1][1]) / (denominator * (port1.wdf.R * y[0][0] + (T) 1.0));
        B = -port1.wdf.R * y[0][1] / (port1.wdf.R * y[0][0] + (T) 1.0);
        C = -y[1][0] / denominator;
    }

    /** Accepts an incident wave into a WDF Y-Parameter. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
        port1.incident (A * port1.wdf.b + B * x);
    }

    /** Propogates a reflected wave from a WDF Y-Parameter. */
    inline T reflected() noexcept
    {
        wdf.b = C * port1.reflected();
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    PortType& port1;
    T y[2][2] = { { (T) 0.0, (T) 0.0 }, { (T) 0.0, (T) 0.0 } };

    T denominator = (T) 1.0;
    T A = (T) 1.0;
    T B = (T) 1.0;
    T C = (T) 1.0;
};

/** WDF Ideal Voltage source (non-adaptable) */
template <typename T, typename Next>
class IdealVoltageSourceT final : public RootWDF
{
public:
    explicit IdealVoltageSourceT (Next& next)
    {
        next.connectToParent (this);
        calcImpedance();
    }

    void calcImpedance() override {}

    /** Sets the voltage of the voltage source, in Volts */
    void setVoltage (T newV) { Vs = newV; }

    /** Accepts an incident wave into a WDF ideal voltage source. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF ideal voltage source. */
    inline T reflected() noexcept
    {
        wdf.b = (T) 0 - wdf.a + (T) 2.0 * Vs;
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    T Vs = (T) 0.0;
};

/** WDF Voltage source with series resistance */
template <typename T>
class ResistiveVoltageSourceT final : public BaseWDF
{
public:
    /** Creates a new resistive voltage source.
         * @param value: initial resistance value, in Ohms
         */
    explicit ResistiveVoltageSourceT (T value = typename WDFMembers<T>::NumericType (1.0e-9)) : R_value (value)
    {
        calcImpedance();
    }

    /** Sets the resistance value of the series resistor, in Ohms. */
    void setResistanceValue (T newR)
    {
        if (newR == R_value)
            return;

        R_value = newR;
        propagateImpedanceChange();
    }

    /** Computes the impedance for a WDF resistive voltage souce
         * Z_Vr = Z_R
         */
    inline void calcImpedance() override
    {
        wdf.R = R_value;
        wdf.G = (T) 1.0 / wdf.R;
    }

    /** Sets the voltage of the voltage source, in Volts */
    void setVoltage (T newV) { Vs = newV; }

    /** Accepts an incident wave into a WDF resistive voltage source. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF resistive voltage source. */
    inline T reflected() noexcept
    {
        wdf.b = Vs;
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    T Vs = (T) 0.0;
    T R_value = (T) 1.0e-9;
};

/** WDF Current source (non-adpatable) */
template <typename T, typename Next>
class IdealCurrentSourceT final : public RootWDF
{
public:
    explicit IdealCurrentSourceT (Next& n) : next (n)
    {
        next.connectToParent (this);
        calcImpedance();
    }

    inline void calcImpedance() override
    {
        twoR = (T) 2.0 * next.wdf.R;
        twoR_Is = twoR * Is;
    }

    /** Sets the current of the current source, in Amps */
    void setCurrent (T newI)
    {
        Is = newI;
        twoR_Is = twoR * Is;
    }

    /** Accepts an incident wave into a WDF ideal current source. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF ideal current source. */
    inline T reflected() noexcept
    {
        wdf.b = twoR_Is + wdf.a;
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    Next& next;

    T Is = (T) 0.0;
    T twoR;
    T twoR_Is;
};

/** WDF Current source with parallel resistance */
template <typename T>
class ResistiveCurrentSourceT final : public BaseWDF
{
public:
    /** Creates a new resistive current source.
         * @param value: initial resistance value, in Ohms
         */
    explicit ResistiveCurrentSourceT (T value = typename WDFMembers<T>::NumericType (1.0e9)) : R_value (value)
    {
        calcImpedance();
    }

    /** Sets the resistance value of the parallel resistor, in Ohms. */
    void setResistanceValue (T newR)
    {
        if (newR == R_value)
            return;

        R_value = newR;
        propagateImpedanceChange();
    }

    /** Computes the impedance for a WDF resistive current souce
         * Z_Ir = Z_R
         */
    inline void calcImpedance() override
    {
        wdf.R = R_value;
        wdf.G = (T) 1.0 / wdf.R;
    }

    /** Sets the current of the current source, in Amps */
    void setCurrent (T newI) { Is = newI; }

    /** Accepts an incident wave into a WDF resistive current source. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF resistive current source. */
    inline T reflected() noexcept
    {
        wdf.b = wdf.R * Is;
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    T Is = (T) 0.0;
    T R_value = (T) 1.0e9;
};

/** Enum to determine which diode approximation eqn. to use */
enum DiodeQuality
{
    Good, // see reference eqn (18)
    Best, // see reference eqn (39)
};

/**
 * WDF diode pair (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
template <typename T, typename Next, DiodeQuality Quality = DiodeQuality::Best>
class DiodePairT final : public RootWDF
{
public:
    /**
     * Creates a new WDF diode pair, with the given diode specifications.
     * @param n: the next element in the WDF connection tree
     * @param Is: reverse saturation current
     * @param Vt: thermal voltage
     * @param nDiodes: the number of series diodes
     */
    DiodePairT (Next& n, T Is, T Vt = typename WDFMembers<T>::NumericType (25.85e-3), T nDiodes = 1) : next (n)
    {
        next.connectToParent (this);
        setDiodeParameters (Is, Vt, nDiodes);
    }

    /** Sets diode specific parameters */
    void setDiodeParameters (T newIs, T newVt, T nDiodes)
    {
        Is = newIs;
        Vt = nDiodes * newVt;
        twoVt = (T) 2 * Vt;
        oneOverVt = (T) 1 / Vt;
        calcImpedance();
    }

    inline void calcImpedance() override
    {
        calcImpedanceInternal();
    }

    /** Accepts an incident wave into a WDF diode pair. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF diode pair. */
    inline T reflected() noexcept
    {
        reflectedInternal();
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    /** Implementation for float/double (Good). */
    template <typename C = T, DiodeQuality Q = Quality>
    inline typename std::enable_if<std::is_floating_point<C>::value && (Q == Good), void>::type
        reflectedInternal() noexcept
    {
        // See eqn (18) from reference paper
        T lambda = (T) signum (wdf.a);
        wdf.b = wdf.a + (T) 2 * lambda * (R_Is - Vt * Omega::omega4 (logR_Is_overVt + lambda * wdf.a * oneOverVt + R_Is_overVt));
    }

    /** Implementation for float/double (Best). */
    template <typename C = T, DiodeQuality Q = Quality>
    inline typename std::enable_if<std::is_floating_point<C>::value && (Q == Best), void>::type
        reflectedInternal() noexcept
    {
        // See eqn (39) from reference paper
        T lambda = (T) signum (wdf.a);
        T lambda_a_over_vt = lambda * wdf.a * oneOverVt;
        wdf.b = wdf.a - twoVt * lambda * (Omega::omega4 (logR_Is_overVt + lambda_a_over_vt) - Omega::omega4 (logR_Is_overVt - lambda_a_over_vt));
    }

    template <typename C = T>
    inline typename std::enable_if<std::is_floating_point<C>::value, void>::type
        calcImpedanceInternal() noexcept
    {
        R_Is = next.wdf.R * Is;
        R_Is_overVt = R_Is * oneOverVt;
        logR_Is_overVt = std::log (R_Is_overVt);
    }

#if WDF_USING_JUCE
    /** Implementation for SIMD float/double (Good). */
    template <typename C = T, DiodeQuality Q = Quality>
    inline typename std::enable_if<(std::is_same<juce::dsp::SIMDRegister<float>, C>::value
                                    || std::is_same<juce::dsp::SIMDRegister<double>, C>::value)
                                       && (Q == Good),
                                   void>::type
        reflectedInternal() noexcept
    {
        // See eqn (18) from reference paper
        T lambda = signumSIMD (wdf.a);
        wdf.b = wdf.a + (T) 2 * lambda * (R_Is - Vt * Omega::omega4 (logR_Is_overVt + lambda * wdf.a * oneOverVt + R_Is_overVt));
    }

    /** Implementation for SIMD float/double (Best). */
    template <typename C = T, DiodeQuality Q = Quality>
    inline typename std::enable_if<(std::is_same<juce::dsp::SIMDRegister<float>, C>::value
                                    || std::is_same<juce::dsp::SIMDRegister<double>, C>::value)
                                       && (Q == Best),
                                   void>::type
        reflectedInternal() noexcept
    {
        // See eqn (39) from reference paper
        T lambda = signumSIMD (wdf.a);
        T lambda_a_over_vt = lambda * wdf.a * oneOverVt;
        wdf.b = wdf.a - twoVt * lambda * (Omega::omega4 (logR_Is_overVt + lambda_a_over_vt) - Omega::omega4 (logR_Is_overVt - lambda_a_over_vt));
    }

    template <typename C = T>
    inline typename std::enable_if<std::is_same<juce::dsp::SIMDRegister<float>, C>::value
                                       || std::is_same<juce::dsp::SIMDRegister<double>, C>::value,
                                   void>::type
        calcImpedanceInternal() noexcept
    {
        R_Is = next.wdf.R * Is;
        R_Is_overVt = R_Is * oneOverVt;
        logR_Is_overVt = logSIMD (R_Is_overVt);
    }
#endif
    T Is; // reverse saturation current
    T Vt; // thermal voltage

    // pre-computed vars
    T twoVt;
    T oneOverVt;
    T R_Is;
    T R_Is_overVt;
    T logR_Is_overVt;

    Next& next;
};

/**
 * WDF diode (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
template <typename T, typename Next, DiodeQuality Quality = DiodeQuality::Best>
class DiodeT final : public RootWDF
{
public:
    /**
     * Creates a new WDF diode, with the given diode specifications.
     * @param n: the next element in the WDF connection tree
     * @param Is: reverse saturation current
     * @param Vt: thermal voltage
     * @param nDiodes: the number of series diodes
     */
    DiodeT (Next& n, T Is, T Vt = typename WDFMembers<T>::NumericType (25.85e-3), T nDiodes = 1) : next (n)
    {
        next.connectToParent (this);
        setDiodeParameters (Is, Vt, nDiodes);
    }

    /** Sets diode specific parameters */
    void setDiodeParameters (T newIs, T newVt, T nDiodes)
    {
        Is = newIs;
        Vt = nDiodes * newVt;
        twoVt = (T) 2 * Vt;
        oneOverVt = (T) 1 / Vt;
        calcImpedance();
    }

    inline void calcImpedance() override
    {
        calcImpedanceInternal();
    }

    /** Accepts an incident wave into a WDF diode. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF diode. */
    inline T reflected() noexcept
    {
        // See eqn (10) from reference paper
        wdf.b = wdf.a + twoR_Is - twoVt * Omega::omega4 (logR_Is_overVt + wdf.a * oneOverVt + R_Is_overVt);
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    /** Implementation for float/double. */
    template <typename C = T>
    inline typename std::enable_if<std::is_floating_point<C>::value, void>::type
        calcImpedanceInternal() noexcept
    {
        twoR_Is = (T) 2 * next.wdf.R * Is;
        R_Is_overVt = next.wdf.R * Is * oneOverVt;
        logR_Is_overVt = std::log (R_Is_overVt);
    }

#if WDF_USING_JUCE
    /** Implementation for SIMD float/double. */
    template <typename C = T>
    inline typename std::enable_if<std::is_same<juce::dsp::SIMDRegister<float>, C>::value
                                       || std::is_same<juce::dsp::SIMDRegister<double>, C>::value,
                                   void>::type
        calcImpedanceInternal() noexcept
    {
        twoR_Is = (T) 2 * next.wdf.R * Is;
        R_Is_overVt = next.wdf.R * Is * oneOverVt;
        logR_Is_overVt = logSIMD (R_Is_overVt);
    }
#endif
    T Is; // reverse saturation current
    T Vt; // thermal voltage

    // pre-computed vars
    T twoVt;
    T oneOverVt;
    T twoR_Is;
    T R_Is_overVt;
    T logR_Is_overVt;

    Next& next;
};

/** WDF Switch (non-adaptable) */
template <typename T, typename Next>
class SwitchT final : public RootWDF
{
public:
    explicit SwitchT (Next& next)
    {
        next.connectToParent (this);
    }

    inline void calcImpedance() override {}

    /** Sets the state of the switch. */
    void setClosed (bool shouldClose) { closed = shouldClose; }

    /** Accepts an incident wave into a WDF switch. */
    inline void incident (T x) noexcept
    {
        wdf.a = x;
    }

    /** Propogates a reflected wave from a WDF switch. */
    inline T reflected() noexcept
    {
        wdf.b = closed ? -wdf.a : wdf.a;
        return wdf.b;
    }

    WDFMembers<T> wdf;

private:
    bool closed = true;
};

//==============================================================

/** Probe the voltage across this circuit element. */
template <typename T, typename WDFType>
inline T voltage (const WDFType& wdf) noexcept
{
    return (wdf.wdf.a + wdf.wdf.b) * (T) 0.5;
}

/**Probe the current through this circuit element. */
template <typename T, typename WDFType>
inline T current (const WDFType& wdf) noexcept
{
    return (wdf.wdf.a - wdf.wdf.b) * ((T) 0.5 * wdf.wdf.G);
}

// useful "factory" functions so you don't have to declare all the template parameters

/** Factory method for creating a parallel adaptor between two elements. */
template <typename T, typename P1Type, typename P2Type>
[[maybe_unused]] WDFParallelT<T, P1Type, P2Type> makeParallel (P1Type& p1, P2Type& p2)
{
    return WDFParallelT<T, P1Type, P2Type> (p1, p2);
}

/** Factory method for creating a series adaptor between two elements. */
template <typename T, typename P1Type, typename P2Type>
[[maybe_unused]] WDFSeriesT<T, P1Type, P2Type> makeSeries (P1Type& p1, P2Type& p2)
{
    return WDFSeriesT<T, P1Type, P2Type> (p1, p2);
}

/** Factory method for creating a polarity inverter. */
template <typename T, typename PType>
[[maybe_unused]] PolarityInverterT<T, PType> makeInverter (PType& p1)
{
    return PolarityInverterT<T, PType> (p1);
}

} // namespace chowdsp::WDFT

#endif // WDF_T_INCLUDED
