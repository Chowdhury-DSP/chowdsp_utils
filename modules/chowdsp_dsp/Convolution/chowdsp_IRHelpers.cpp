#include "chowdsp_IRHelpers.h"

namespace chowdsp::IRHelpers
{
template <typename TransformFunc>
void transformIRFreqDomain (float* targetIR, const float* originalIR, int numSamples, juce::dsp::FFT& fft, TransformFunc&& transformer, bool removeDCBias = true, bool normalizeRMS = true)
{
    // The FFT object is initialized for the wrong size IR!
    jassert (fft.getSize() == numSamples);

    // save this now, in case originalIR data gets overwritten later
    const auto hRMS = FloatVectorOperations::computeRMS (originalIR, numSamples);

    // memory allocations!
    std::vector<std::complex<float>> timeDomainData ((size_t) numSamples, std::complex<float> {});
    std::vector<std::complex<float>> freqDomainData ((size_t) numSamples, std::complex<float> {});

    // perform forward FFT
    std::copy (originalIR, originalIR + numSamples, timeDomainData.begin());
    fft.perform (timeDomainData.data(), freqDomainData.data(), false);

    transformer (freqDomainData);

    // perform inverse FFT
    fft.perform (freqDomainData.data(), timeDomainData.data(), true);
    std::transform (timeDomainData.begin(), timeDomainData.end(), targetIR, [] (auto x)
                    { return std::real (x); });

    if (removeDCBias) // remove DC offset
    {
        const auto hLinMean = FloatVectorOperations::accumulate (targetIR, numSamples) / (float) numSamples;
        juce::FloatVectorOperations::add (targetIR, -hLinMean, numSamples);
    }

    if (normalizeRMS) // normalize
    {
        const auto hLinRMS = FloatVectorOperations::computeRMS (targetIR, numSamples);
        juce::FloatVectorOperations::multiply (targetIR, hRMS / hLinRMS, numSamples);
    }
}

void makeLinearPhase (float* linearPhaseIR, const float* originalIR, int numSamples, juce::dsp::FFT& fft)
{
    transformIRFreqDomain (linearPhaseIR,
                           originalIR,
                           numSamples,
                           fft,
                           [numSamples] (auto& freqDomainData)
                           {
                               // compute delay kernels
                               std::vector<std::complex<float>> delayKernels ((size_t) numSamples, std::complex<float> {});
                               const auto phaseIncrement = juce::MathConstants<float>::twoPi / float (numSamples - 1);
                               std::generate (
                                   delayKernels.begin(),
                                   delayKernels.end(),
                                   [numSamples, n = 0.0f, phaseIncrement]() mutable
                                   {
                                       using namespace std::complex_literals;
                                       return std::exp (-1.0if * ((float) numSamples / 2) * (n++ * phaseIncrement));
                                   });

                               // compute frequeny domain linear phase IR
                               std::transform (
                                   freqDomainData.begin(),
                                   freqDomainData.end(),
                                   delayKernels.begin(),
                                   freqDomainData.begin(),
                                   [] (auto H, auto phi)
                                   { return phi * std::abs (H); });
                           });
}

void makeMinimumPhase (float* minimumPhaseIR, const float* originalIR, int numSamples, juce::dsp::FFT& fft)
{
    transformIRFreqDomain (
        minimumPhaseIR,
        originalIR,
        numSamples,
        fft,
        [numSamples, &fft] (auto& freqDomainData)
        {
            auto hilbert = [&fft] (std::complex<float>* output, const std::complex<float>* input, int nSamples)
            {
                std::vector<std::complex<float>> H ((size_t) nSamples, std::complex<float> {});
                fft.perform (input, H.data(), false);

                using namespace std::complex_literals;
                const auto halfN = nSamples / 2;
                const auto modN = nSamples % 2;

                H[0] = {};
                if (modN == 0)
                    H[(size_t) halfN] = {};

                std::transform (H.begin(), H.begin() + halfN + modN, H.begin(), [] (auto X)
                                { return 1.0if * X; });
                std::transform (H.begin() + halfN + 1, H.end(), H.begin() + halfN + 1, [] (auto X)
                                { return -1.0if * X; });

                fft.perform (H.data(), output, true);
            };

            std::transform (
                freqDomainData.begin(),
                freqDomainData.end(),
                freqDomainData.begin(),
                [] (auto H)
                { return std::abs (H); });

            std::vector<std::complex<float>> arg_H ((size_t) numSamples, std::complex<float> {});
            std::transform (
                freqDomainData.begin(),
                freqDomainData.end(),
                arg_H.begin(),
                [] (auto H)
                { return std::log (H); });

            hilbert (arg_H.data(), arg_H.data(), numSamples);

            std::transform (
                freqDomainData.begin(),
                freqDomainData.end(),
                arg_H.begin(),
                freqDomainData.begin(),
                [] (auto H, auto arg_H_val)
                {
                    using namespace std::complex_literals;
                    return H * std::exp (1.0if * arg_H_val);
                });
        },
        false,
        false);
}
} // namespace chowdsp::IRHelpers
