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

    resampledBuffer.setSize ((int) spec.numChannels, resampledMaxBlockSize);
}

template <typename ResamplingType>
void RepitchedSource<ResamplingType>::reset()
{
    resampler.reset();
    resetRepitched();
}

template <typename ResamplingType>
AudioBlock<float> RepitchedSource<ResamplingType>::process (int numSamples) noexcept
{
    const auto resampleRatio = 1.0f / resampler.getResampleRatio();
    auto resampledNumSamples = (int) std::ceil (resampleRatio * (float) numSamples);
    resampledBuffer.setSize (resampledBuffer.getNumChannels(), resampledNumSamples, false, false, true);
    resampledBuffer.clear();

    auto&& block = AudioBlock<float> { resampledBuffer };
    processRepitched (block);
    return resampler.process (block).toAudioBlock();
}
} // namespace chowdsp
