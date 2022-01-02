#ifndef WDF_H_INCLUDED
#define WDF_H_INCLUDED

#include <string>
#include <utility>
#include "wdf_t.h"

/** API for constructing Wave Digital Filters with run-time flexibility */
namespace chowdsp::WDF
{
#if WDF_USING_JUCE
using namespace SIMDUtils;
#endif // WDF_USING_JUCE

/** Wave digital filter base class */
template <typename T>
class WDF : public WDFT::BaseWDF
{
    using NumericType = typename WDFT::WDFMembers<T>::NumericType;

public:
    explicit WDF (std::string type) : type (std::move (type)) {}
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
        return (wdf.a + wdf.b) / (T) 2.0;
    }

    /**Probe the current through this circuit element. */
    inline T current() const noexcept
    {
        return (wdf.a - wdf.b) / ((T) 2.0 * wdf.R);
    }

    // These classes need access to a,b
    template <typename>
    friend class YParameter;

    template <typename>
    friend class WDFParallel;

    template <typename>
    friend class WDFSeries;

    WDFT::WDFMembers<T> wdf;

private:
    const std::string type;

    WDF<T>* parent = nullptr;
};

template <typename T, typename WDFType>
class WDFWrapper : public WDF<T>
{
public:
    template <typename... Args>
    explicit WDFWrapper (std::string name, Args&&... args) : WDF<T> (name),
                                                             internalWDF (std::forward<Args> (args)...)
    {
        calcImpedance();
    }

    /** Computes the impedance of the WDF resistor, Z_R = R. */
    inline void calcImpedance() override
    {
        internalWDF.calcImpedance();
        this->wdf.R = internalWDF.wdf.R;
        this->wdf.G = internalWDF.wdf.G;
    }

    /** Accepts an incident wave into a WDF resistor. */
    inline void incident (T x) noexcept override
    {
        this->wdf.a = x;
        internalWDF.incident (x);
    }

    /** Propogates a reflected wave from a WDF resistor. */
    inline T reflected() noexcept override
    {
        this->wdf.b = internalWDF.reflected();
        return this->wdf.b;
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
    explicit Resistor (T value) : WDFWrapper<T, WDFT::ResistorT<T>> ("Resistor", value)
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
    explicit Capacitor (T value, T fs = (T) 48000.0) : WDFWrapper<T, WDFT::CapacitorT<T>> ("Capacitor", value, fs)
    {
    }

    /** Sets the capacitance value of the WDF capacitor, in Farads. */
    void setCapacitanceValue (T newC)
    {
        this->internalWDF.setCapacitanceValue (newC);
        this->propagateImpedance();
    }

    /** Prepares the capacitor to operate at a new sample rate */
    void prepare (T sampleRate)
    {
        this->internalWDF.prepare (sampleRate);
        this->propagateImpedance();
    }

    /** Resets the capacitor state */
    void reset()
    {
        this->internalWDF.reset();
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
    explicit CapacitorAlpha (T value, T fs = (T) 48000.0, T alpha = (T) 1.0) : WDFWrapper<T, WDFT::CapacitorAlphaT<T>> ("Capacitor", value, fs, alpha)
    {
    }

    /** Sets the capacitance value of the WDF capacitor, in Farads. */
    void setCapacitanceValue (T newC)
    {
        this->internalWDF.setCapacitanceValue (newC);
        this->propagateImpedance();
    }

    /** Prepares the capacitor to operate at a new sample rate */
    void prepare (T sampleRate)
    {
        this->internalWDF.prepare (sampleRate);
        this->propagateImpedance();
    }

    /** Resets the capacitor state */
    void reset()
    {
        this->internalWDF.reset();
    }

    /** Sets a new alpha value to use for the alpha transform */
    void setAlpha (T newAlpha)
    {
        this->internalWDF.setAlpha (newAlpha);
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
    explicit Inductor (T value, T fs = (T) 48000.0) : WDFWrapper<T, WDFT::InductorT<T>> ("Inductor", value, fs)
    {
    }

    /** Sets the inductance value of the WDF inductor, in Henries. */
    void setInductanceValue (T newL)
    {
        this->internalWDF.setInductanceValue (newL);
        this->propagateImpedance();
    }

    /** Prepares the inductor to operate at a new sample rate */
    void prepare (T sampleRate)
    {
        this->internalWDF.prepare (sampleRate);
        this->propagateImpedance();
    }

    /** Resets the inductor state */
    void reset()
    {
        this->internalWDF.reset();
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
    explicit InductorAlpha (T value, T fs = 48000.0, T alpha = 1.0) : WDFWrapper<T, WDFT::InductorAlphaT<T>> ("Inductor", value, fs, alpha)
    {
    }

    /** Sets the inductance value of the WDF inductor, in Henries. */
    void setInductanceValue (T newL)
    {
        this->internalWDF.setInductanceValue (newL);
        this->propagateImpedance();
    }

    /** Prepares the inductor to operate at a new sample rate */
    void prepare (T sampleRate)
    {
        this->internalWDF.prepare (sampleRate);
        this->propagateImpedance();
    }

    /** Resets the inductor state */
    void reset()
    {
        this->internalWDF.reset();
    }

    /** Sets a new alpha value to use for the alpha transform */
    void setAlpha (T newAlpha)
    {
        this->internalWDF.setAlpha (newAlpha);
        this->propagateImpedance();
    }
};

/** WDF Switch (non-adaptable) */
template <typename T>
class Switch final : public WDFWrapper<T, WDFT::SwitchT<T, WDF<T>>>
{
public:
    explicit Switch (WDF<T>* next) : WDFWrapper<T, WDFT::SwitchT<T, WDF<T>>> ("Switch", *next)
    {
        next->connectToNode (this);
    }

    /** Sets the state of the switch. */
    void setClosed (bool shouldClose)
    {
        this->internalWDF.setClosed (shouldClose);
    }
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
    explicit PolarityInverter (WDF<T>* port1) : WDFWrapper<T, WDFT::PolarityInverterT<T, WDF<T>>> ("Polarity Inverter", *port1)
    {
        port1->connectToNode (this);
    }
};

/** WDF y-parameter 2-port (short circuit admittance) */
template <typename T>
class YParameter final : public WDFWrapper<T, WDFT::YParameterT<T, WDF<T>>>
{
public:
    YParameter (WDF<T>* port1, T y11, T y12, T y21, T y22) : WDFWrapper<T, WDFT::YParameterT<T, WDF<T>>> ("YParameter", *port1, y11, y12, y21, y22)
    {
        port1->connectToNode (this);
    }
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
    using NumericType = typename WDFT::WDFMembers<T>::NumericType;

public:
    /** Creates a new resistive voltage source.
     * @param value: initial resistance value, in Ohms
     */
    explicit ResistiveVoltageSource (T value = (NumericType) 1.0e-9) : WDFWrapper<T, WDFT::ResistiveVoltageSourceT<T>> ("Resistive Voltage", value)
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
    explicit IdealVoltageSource (WDF<T>* next) : WDFWrapper<T, WDFT::IdealVoltageSourceT<T, WDF<T>>> ("IdealVoltage", *next)
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
    using NumericType = typename WDFT::WDFMembers<T>::NumericType;

public:
    /** Creates a new resistive current source.
     * @param value: initial resistance value, in Ohms
     */
    explicit ResistiveCurrentSource (T value = (NumericType) 1.0e9) : WDFWrapper<T, WDFT::ResistiveCurrentSourceT<T>> ("Resistive Current", value)
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
    explicit IdealCurrentSource (WDF<T>* next) : WDFWrapper<T, WDFT::IdealCurrentSourceT<T, WDF<T>>> ("Ideal Current", *next)
    {
        next->connectToNode (this);
    }

    /** Sets the current of the current source, in Amps */
    void setCurrent (T newI) { this->internalWDF.setCurrent (newI); }
};

/**
 * WDF diode pair (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
template <typename T, WDFT::DiodeQuality Q = WDFT::DiodeQuality::Best>
class DiodePair final : public WDFRootWrapper<T, WDFT::DiodePairT<T, WDF<T>, Q>>
{
    using NumericType = typename WDFT::WDFMembers<T>::NumericType;

public:
    /**
     * Creates a new WDF diode pair, with the given diode specifications.
     * @param next: the next element in the WDF connection tree
     * @param Is: reverse saturation current
     * @param Vt: thermal voltage
     * @param nDiodes: the number of series diodes
     */
    DiodePair (WDF<T>* next, T Is, T Vt = (NumericType) 25.85e-3, T nDiodes = (T) 1) : WDFRootWrapper<T, WDFT::DiodePairT<T, WDF<T>, Q>> ("DiodePair", *next, *next, Is, Vt, nDiodes)
    {
        next->connectToNode (this);
    }

    /** Sets diode specific parameters */
    void setDiodeParameters (T newIs, T newVt, T nDiodes)
    {
        this->internalWDF.setDiodeParameters (newIs, newVt, nDiodes);
    }
};

/**
 * WDF diode (non-adaptable)
 * See Werner et al., "An Improved and Generalized Diode Clipper Model for Wave Digital Filters"
 * https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
template <typename T>
class Diode final : public WDFRootWrapper<T, WDFT::DiodeT<T, WDF<T>>>
{
    using NumericType = typename WDFT::WDFMembers<T>::NumericType;

public:
    /**
     * Creates a new WDF diode, with the given diode specifications.
     * @param next: the next element in the WDF connection tree
     * @param Is: reverse saturation current
     * @param Vt: thermal voltage
     * @param nDiodes: the number of series diodes
     */
    Diode (WDF<T>* next, T Is, T Vt = (NumericType) 25.85e-3, T nDiodes = 1) : WDFRootWrapper<T, WDFT::DiodeT<T, WDF<T>>> ("Diode", *next, *next, Is, Vt, nDiodes)
    {
        next->connectToNode (this);
    }

    /** Sets diode specific parameters */
    void setDiodeParameters (T newIs, T newVt, T nDiodes)
    {
        this->internalWDF.setDiodeParameters (newIs, newVt, nDiodes);
    }
};

} // namespace chowdsp::WDF

#undef WDF_USING_JUCE

#endif // WDF_H_INCLUDED
