#pragma once

namespace chowdsp
{
/** Helper methods for working with impulse responses */
namespace IRHelpers
{
    /**
     * Transforms an impulse response into a linear phase impulse response
     *
     * Note that this method allocates memory, so it should not be called on the audio thread!
     *
     * @param linearPhaseIR     A "destination" pointer to the linear phase IR data
     * @param originalIR        A "source" pointer to the original IR data
     * @param numSamples        The length of the impulse response in samples
     * @param fft               A shared FFT object to use for the computation
     */
    void makeLinearPhase (float* linearPhaseIR, const float* originalIR, int numSamples, const juce::dsp::FFT& fft);

    /**
     * Transforms an impulse response into a minimum phase impulse response
     *
     * Note that this method allocates memory, so it should not be called on the audio thread!
     *
     * @param minimumPhaseIR    A "destination" pointer to the minimum phase IR data
     * @param originalIR        A "source" pointer to the original IR data
     * @param numSamples        The length of the impulse response in samples
     * @param fft               A shared FFT object to use for the computation
     */
    void makeMinimumPhase (float* minimumPhaseIR, const float* originalIR, int numSamples, const juce::dsp::FFT& fft);

    /**
     * Transforms an impulse response to have exactly half the magnitude response of the original.
     *
     * This can be useful for creating a linear phase IR from a prototype filter. Just run an
     * impulse through the filter forwards and backwards, then use this method to get half the
     * magnitude response.
     *
     * @param halfMagIR     A "destination" pointer to the half magnitude IR data
     * @param originalIR    A "source" pointer to the original IR data
     * @param numSamples    The length of the impulse response in samples
     * @param fft           A shared FFT object to use for the computation
     */
    void makeHalfMagnitude (float* halfMagIR, const float* originalIR, int numSamples, const juce::dsp::FFT& fft);

} // namespace IRHelpers
} // namespace chowdsp
