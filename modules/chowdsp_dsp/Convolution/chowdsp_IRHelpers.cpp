#include "chowdsp_IRHelpers.h"

namespace chowdsp::IRHelpers
{
void makeLinearPhase (float* linearPhaseIR, const float* originalIR, int numSamples, juce::dsp::FFT& fft)
{
    // The FFT object is initialized for the wrong size IR!
    jassert (fft.getSize() == numSamples);

    // save this now, in case originalIR data gets overwritten later
    const auto hRMS = FloatVectorOperations::computeRMS (originalIR, numSamples);

    // memory allocations!
    std::vector<std::complex<float>> timeDomainData ((size_t) numSamples, std::complex<float> {});
    std::vector<std::complex<float>> freqDomainData ((size_t) numSamples, std::complex<float> {});
    std::vector<std::complex<float>> delayKernels ((size_t) numSamples, std::complex<float> {});

    // perform forward FFT
    std::copy (originalIR, originalIR + numSamples, timeDomainData.begin());
    fft.perform (timeDomainData.data(), freqDomainData.data(), false);

    // compute delay kernels
    const auto phaseIncrement = juce::MathConstants<float>::twoPi / float (numSamples - 1);
    std::generate (
        delayKernels.begin(),
        delayKernels.end(),
        [numSamples, n = 0.0f, phaseIncrement]() mutable {
            using namespace std::complex_literals;
            return std::exp (-1.0if * ((float) numSamples / 2) * (n++ * phaseIncrement));
        });

    // compute frequeny domain linear phase IR
    std::transform (
        freqDomainData.begin(),
        freqDomainData.end(),
        delayKernels.begin(),
        freqDomainData.begin(),
        [] (auto H, auto phi) { return phi * std::abs (H); });

    // perform inverse FFT
    fft.perform (freqDomainData.data(), timeDomainData.data(), true);
    std::transform (timeDomainData.begin(), timeDomainData.end(), linearPhaseIR, [] (auto x) { return std::real (x); });

    // remove DC offset
    const auto hLinMean = FloatVectorOperations::accumulate (linearPhaseIR, numSamples) / (float) numSamples;
    juce::FloatVectorOperations::add (linearPhaseIR, -hLinMean, numSamples);

    // normalize
    const auto hLinRMS = FloatVectorOperations::computeRMS (linearPhaseIR, numSamples);
    juce::FloatVectorOperations::multiply (linearPhaseIR, hRMS / hLinRMS, numSamples);
}
} // namespace chowdsp::IRHelpers
