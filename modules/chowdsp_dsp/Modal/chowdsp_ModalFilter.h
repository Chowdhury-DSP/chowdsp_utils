#pragma once

#include <complex>

namespace chowdsp
{
/** Max Mathews phasor filter, a recursive filter
 * that filters a single frequency, with a given
 * damping factor and complex amplitude. This filter
 * is guaranteed stable, and reacts well to real-time
 * parameter changes. For more information, see:
 * https://ccrma.stanford.edu/~jos/smac03maxjos/,
 * and http://dafx2019.bcu.ac.uk/papers/DAFx2019_paper_48.pdf.
 */
template <typename T>
class ModalFilter
{
public:
    ModalFilter() = default;

    /** Prepare the filter to process the audio at a given sample rate */
    virtual void prepare (T sampleRate);

    /** reset filter state */
    virtual inline void reset() noexcept { y1 = std::complex<T> { (T) 0.0 }; }

    /** Sets the complex amplitude of the filter */
    virtual inline void setAmp (std::complex<T> amp) noexcept { amplitude = amp; }

    /** Sets the amplitude and phase of the filter */
    virtual inline void setAmp (T amp, T phase) noexcept { amplitude = std::polar (amp, phase); }

    /** Sets the decay characteristic of the filter, in units of T60 */
    virtual inline void setDecay (T newT60) noexcept
    {
        t60 = newT60;
        decayFactor = calcDecayFactor();
        updateParams();
    }

    /** Sets the resonant frequency of the filter */
    virtual inline void setFreq (T newFreq) noexcept
    {
        freq = newFreq;
        oscCoef = calcOscCoef();
        updateParams();
    }

    /** Process a single sample */
    virtual inline T processSample (T x)
    {
        auto y = filtCoef * y1 + amplitude * x;
        y1 = y;
        return std::imag (y);
    }

    /** Process a block of samples */
    virtual void processBlock (T* buffer, const int numSamples);

protected:
    inline void updateParams() noexcept { filtCoef = decayFactor * oscCoef; }

    inline T calcDecayFactor() noexcept
    {
        return std::pow ((T) 0.001, (T) 1 / (t60 * fs));
    }

    inline std::complex<T> calcOscCoef() noexcept
    {
        constexpr std::complex<T> jImag { 0, 1 };
        return std::exp (jImag * juce::MathConstants<T>::twoPi * (freq / fs));
    }

    std::complex<T> filtCoef = 0;
    T decayFactor = 0;
    std::complex<T> oscCoef = 0;

    std::complex<T> y1 = 0; // filter state

    T freq = 1;
    T t60 = 1;
    std::complex<T> amplitude;
    T fs = 44100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModalFilter)
};

//=========================================================================
/** An implementation of the modal filter parallelised with SIMDComplex. */
template <typename FloatType>
class ModalFilter<juce::dsp::SIMDRegister<FloatType>>
{
    using VType = juce::dsp::SIMDRegister<FloatType>;
    using CType = SIMDUtils::SIMDComplex<FloatType>;

public:
    ModalFilter() = default;

    /** Prepare the filter to process the audio at a given sample rate */
    virtual void prepare (FloatType sampleRate);

    /** reset filter state */
    virtual inline void reset() noexcept { y1 = CType { (FloatType) 0, (FloatType) 0 }; }

    /** Sets the complex amplitude of the filter */
    virtual inline void setAmp (VType amp) noexcept { amplitude = amp; }

    /** Sets the amplitude and phase of the filter */
    virtual inline void setAmp (VType amp, VType phase) noexcept { amplitude = CType::polar (amp, phase); }

    /** Sets the decay characteristic of the filter, in units of T60 */
    virtual inline void setDecay (VType newT60) noexcept
    {
        t60 = newT60;
        decayFactor = calcDecayFactor();
        updateParams();
    }

    /** Sets the resonant frequency of the filter */
    virtual inline void setFreq (VType newFreq) noexcept
    {
        freq = newFreq;
        oscCoef = calcOscCoef();
        updateParams();
    }

    /** Process a single sample */
    virtual inline VType processSample (VType x)
    {
        auto y = filtCoef * y1 + amplitude * x;
        y1 = y;
        return y.imag();
    }

    /** Process a block of samples */
    virtual void processBlock (VType* buffer, const int numSamples);

protected:
    inline void updateParams() noexcept { filtCoef = decayFactor * oscCoef; }

    inline VType calcDecayFactor() noexcept
    {
        using namespace SIMDUtils;
        return powSIMD ((VType) (FloatType) 0.001, (VType) 1 / (t60 * fs));
    }

    inline CType calcOscCoef() noexcept
    {
        using namespace SIMDUtils;
        return CType::exp ((freq / fs) * juce::MathConstants<FloatType>::twoPi);
    }

    CType filtCoef { (FloatType) 0, (FloatType) 0 };
    VType decayFactor = 0;
    CType oscCoef { (FloatType) 0, (FloatType) 0 };

    CType y1 = { (FloatType) 0, (FloatType) 0 }; // filter state

    VType freq = 1;
    VType t60 = 1;
    CType amplitude;
    FloatType fs = 44100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModalFilter)
};

} // namespace chowdsp
