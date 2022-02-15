#pragma once

/** Helper methods for working with impulse responses */
namespace chowdsp::IRHelpers
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
void makeLinearPhase (float* linearPhaseIR, const float* originalIR, int numSamples, juce::dsp::FFT& fft);
}
