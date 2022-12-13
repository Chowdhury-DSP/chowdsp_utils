#pragma once

namespace chowdsp
{
/**
 * Max Mathews phasor filter, a recursive filter
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
    virtual ~ModalFilter() = default;

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

    /** Returns the current filter frequency */
    [[nodiscard]] inline T getFreq() const noexcept { return freq; }

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

#if ! CHOWDSP_NO_XSIMD
//=========================================================================
/** An implementation of the modal filter parallelised with xsimd::batch<std::complex>. */
template <typename FloatType>
class ModalFilter<xsimd::batch<FloatType>>
{
    using VType = xsimd::batch<FloatType>;
    using CType = xsimd::batch<std::complex<FloatType>>;

public:
    ModalFilter() = default;
    virtual ~ModalFilter() = default;

    /** Prepare the filter to process the audio at a given sample rate */
    virtual void prepare (FloatType sampleRate);

    /** reset filter state */
    virtual inline void reset() noexcept { y1 = CType {}; }

    /** Sets the scalar amplitude of the filter */
    virtual inline void setAmp (VType amp) noexcept { amplitude = CType { amp, xsimd::batch ((FloatType) 0) }; }

    /** Sets the scalar amplitude of the filter */
    virtual inline void setAmp (CType amp) noexcept { amplitude = amp; }

    /** Sets the amplitude and phase of the filter */
    virtual inline void setAmp (VType amp, VType phase) noexcept { amplitude = SIMDUtils::polar (amp, phase); }

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

    /** Returns the current filter frequency */
    [[nodiscard]] inline VType getFreq() const noexcept { return freq; }

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
        return xsimd::pow ((VType) (FloatType) 0.001, (VType) 1 / (t60 * fs));
    }

    inline CType calcOscCoef() noexcept
    {
        return SIMDUtils::polar ((freq / fs) * juce::MathConstants<FloatType>::twoPi);
    }

    CType filtCoef {};
    VType decayFactor = 0;
    CType oscCoef {};

    CType y1 = {}; // filter state

    VType freq = 1;
    VType t60 = 1;
    CType amplitude;
    FloatType fs = 44100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModalFilter)
};
#endif // ! CHOWDSP_NO_XSIMD
} // namespace chowdsp
