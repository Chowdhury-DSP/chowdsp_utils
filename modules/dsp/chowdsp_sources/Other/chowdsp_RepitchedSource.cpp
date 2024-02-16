#include "chowdsp_RepitchedSource.h"

namespace chowdsp
{
template <typename ResamplingType>
RepitchedSource<ResamplingType>::RepitchedSource (float maxPitchChangeFactor) : maxRepitchFactor (maxPitchChangeFactor)
{
}

template <typename ResamplingType>
void RepitchedSource<ResamplingType>::setRepitchFactor (float newRepitchFactor)
{
    jassert (newRepitchFactor <= maxRepitchFactor && newRepitchFactor >= (1.0f / maxRepitchFactor));
    resampler.setResampleRatio (1.0f / newRepitchFactor);
}

template <typename ResamplingType>
void RepitchedSource<ResamplingType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    resampler.prepare (spec);

    const auto resampledMaxBlockSize = (int) std::ceil (maxRepitchFactor * (float) spec.maximumBlockSize);
    prepareRepitched ({ spec.sampleRate, (juce::uint32) resampledMaxBlockSize, spec.numChannels });

    resampledBuffer.setMaxSize ((int) spec.numChannels, resampledMaxBlockSize);
}

template <typename ResamplingType>
void RepitchedSource<ResamplingType>::reset()
{
    resampler.reset();
    resetRepitched();
}

template <typename ResamplingType>
BufferView<float> RepitchedSource<ResamplingType>::process (int numSamples) noexcept
{
    const auto resampleRatio = 1.0f / resampler.getResampleRatio();
    auto resampledNumSamples = (int) std::ceil (resampleRatio * (float) numSamples);
    resampledBuffer.setCurrentSize (resampledBuffer.getNumChannels(), resampledNumSamples);
    resampledBuffer.clear();

    processRepitched (resampledBuffer);
    return resampler.process (resampledBuffer);
}
} // namespace chowdsp
