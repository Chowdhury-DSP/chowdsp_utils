#include "chowdsp_IRHelpers.h"

namespace chowdsp::IRHelpers
{
template <typename TransformFunc>
void transformIRFreqDomain (float* targetIR, const float* originalIR, int numSamples, const juce::dsp::FFT& fft, TransformFunc&& transformer, bool removeDCBias, bool normalizeRMS)
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

void makeLinearPhase (float* linearPhaseIR, const float* originalIR, int numSamples, const juce::dsp::FFT& fft)
{
    // @TODO: make sure all four types are handled correctly (even/symmetric, odd/symmetric, even/asym, odd/asym), plus add options for symmetric

    transformIRFreqDomain (
        linearPhaseIR,
        originalIR,
        numSamples,
        fft,
        [numSamples] (auto& freqDomainData)
        {
            // compute delay kernels
            std::vector<std::complex<float>> delayKernels ((size_t) numSamples, std::complex<float> {});
            const auto phaseIncrement = juce::MathConstants<float>::twoPi / float (numSamples - 1);
            for (size_t n = 0; n < delayKernels.size(); ++n)
            {
                using namespace std::complex_literals;
                delayKernels[n] = std::exp (-1.0if * ((float) numSamples / 2) * ((float) n * phaseIncrement));
            }

            // compute frequency domain linear phase IR
            std::transform (
                freqDomainData.begin(),
                freqDomainData.end(),
                delayKernels.begin(),
                freqDomainData.begin(),
                [] (auto H, auto phi)
                { return phi * std::abs (H); });
        },
        true,
        true);
}

void makeMinimumPhase (float* minimumPhaseIR, const float* originalIR, int numSamples, const juce::dsp::FFT& fft)
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

                const auto halfN = nSamples / 2;
                const auto modN = nSamples % 2;

                H[0] = std::complex<float> {};
                if (modN == 0)
                    H[(size_t) halfN] = std::complex<float> {};

                std::transform (H.begin(),
                                H.begin() + halfN + modN,
                                H.begin(),
                                [] (auto X)
                                {
                                    using namespace std::complex_literals;
                                    return 1.0if * X;
                                });
                std::transform (H.begin() + halfN + 1,
                                H.end(),
                                H.begin() + halfN + 1,
                                [] (auto X)
                                {
                                    using namespace std::complex_literals;
                                    return -1.0if * X;
                                });

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

void makeHalfMagnitude (float* halfMagIR, const float* originalIR, int numSamples, const juce::dsp::FFT& fft)
{
    transformIRFreqDomain (
        halfMagIR,
        originalIR,
        numSamples,
        fft,
        [] (auto& freqDomainData)
        {
            std::transform (
                freqDomainData.begin(),
                freqDomainData.end(),
                freqDomainData.begin(),
                [] (auto H)
                {
                    using namespace std::complex_literals;
                    return std::sqrt (std::abs (H)) * std::exp (1.0if * std::arg (H));
                });
        },
        false,
        false);
}
} // namespace chowdsp::IRHelpers
