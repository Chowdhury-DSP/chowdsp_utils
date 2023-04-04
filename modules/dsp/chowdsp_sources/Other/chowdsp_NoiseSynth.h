#pragma once

namespace chowdsp
{
/** A subtractive noise synthesizer, with a filter defined by frequency-domain magnitudes. */
template <int nFilterCoeffs = 128>
class NoiseSynth
{
    static_assert (Math::isPowerOfTwo (nFilterCoeffs));

public:
    NoiseSynth() = default;

    /** Prepares the synth to process an audio stream. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        noiseSynthSource.prepare (spec);
        noiseSynthSource.setNoiseType (Noise<float>::NoiseType::Uniform);
        noiseSynthSource.setGainLinear (1.0f);
        noiseSynthFilter.prepare ((int) spec.numChannels);
        noiseSynthFilter.setOrder (nFilterCoeffs);
    }

    /**
     * Sets the synth filter frequency domain magnitudes.
     * There must be the same number magnitudes as there are filter coefficients.
     */
    void setMagnitudes (nonstd::span<const float> magnitudes) noexcept
    {
        jassert (magnitudes.size() == (size_t) nFilterCoeffs);

        // freq. domain -> time domain
        std::copy (magnitudes.begin(), magnitudes.end(), reinterpret_cast<std::complex<float>*> (fftInOut.data()));
        fft.performRealOnlyInverseTransform (fftInOut.data());

        // apply window + make causal
        juce::FloatVectorOperations::multiply (irData.data(), window.data(), fftInOut.data(), nFilterCoeffs);
        FloatVectorOperations::rotate (irData.data(), nFilterCoeffs / 2, (int) irData.size(), fftInOut.data()); // using fftInOut as scratch data for the rotation

        // these coefficients are linear phase... maybe we should try to do some sort of latency compensation?
        noiseSynthFilter.setCoefficients (irData.data());
    }

    /** Generates a buffer of audio. */
    void process (const BufferView<float>& buffer) noexcept
    {
        jassert (buffer.getNumChannels() == 1);

        buffer.clear();
        noiseSynthSource.processBlock (buffer);
        noiseSynthFilter.processBlock (buffer);
    }

    /** Sets the frequency domain magnitudes and generates a buffer of audio. */
    void process (const BufferView<float>& buffer, nonstd::span<const float> magnitudes) noexcept
    {
        setMagnitudes (magnitudes);
        process (buffer);
    }

private:
    static constexpr auto fftOrder = Math::log2<nFilterCoeffs>();
    juce::dsp::FFT fft { fftOrder };

    const std::array<float, (size_t) nFilterCoeffs> window = []
    {
        std::array<float, (size_t) nFilterCoeffs> w;
        // basic Hann window...
        for (auto [idx, val] : enumerate (w))
            val = 0.5f * (1.0f - std::cos (juce::MathConstants<float>::twoPi * (float) idx / (float) nFilterCoeffs));
        // ... rotated for zero-phase
        std::rotate (w.begin(), w.begin() + w.size() / 2, w.end());
        return w;
    }();

    std::array<float, (size_t) nFilterCoeffs * 2> fftInOut;
    std::array<float, (size_t) nFilterCoeffs> irData;

    Noise<float> noiseSynthSource;
    FIRFilter<float> noiseSynthFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseSynth)
};
} // namespace chowdsp
