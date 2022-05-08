#include "chowdsp_Diffuser.h"

namespace chowdsp::Reverb
{
inline double DefaultDiffuserConfig::getDelayMult (int channelIndex, int nChannels)
{
    const auto rangeLow = (double) channelIndex / (double) nChannels;
    const auto rangeHigh = double (channelIndex + 1) / (double) nChannels;
    return rangeLow + juce::Random::getSystemRandom().nextDouble() * (rangeHigh - rangeLow);
}

inline double DefaultDiffuserConfig::getPolarityMultiplier (int /*channelIndex*/, int /*nChannels*/)
{
    return juce::Random::getSystemRandom().nextBool() ? 1.0 : -1.0;
}

inline void DefaultDiffuserConfig::fillChannelSwapIndexes (size_t* indexes, int numChannels)
{
    std::random_device rd;
    std::mt19937 g (rd());

    std::iota (indexes, indexes + numChannels, 0);
    std::shuffle (indexes, indexes + numChannels, g);
}

//======================================================================
template <typename FloatType, int nChannels, typename DelayInterpType>
template <typename DiffuserConfig>
void Diffuser<FloatType, nChannels, DelayInterpType>::prepare (double sampleRate)
{
    fs = (FloatType) sampleRate;

    DiffuserConfig::fillChannelSwapIndexes (channelSwapIndexes.data(), nChannels);

    for (size_t i = 0; i < (size_t) nChannels; ++i)
    {
        delays[i].prepare ({ sampleRate, 128, 1 });
        delayRelativeMults[i] = (FloatType) DiffuserConfig::getDelayMult ((int) i, nChannels);
        polarityMultipliers[i] = (FloatType) DiffuserConfig::getPolarityMultiplier ((int) i, nChannels);
    }
}

template <typename FloatType, int nChannels, typename DelayInterpType>
void Diffuser<FloatType, nChannels, DelayInterpType>::reset()
{
    for (auto& delay : delays)
        delay.reset();
}

template <typename FloatType, int nChannels, typename DelayInterpType>
void Diffuser<FloatType, nChannels, DelayInterpType>::setDiffusionTimeMs (FloatType diffusionTimeMs)
{
    for (size_t i = 0; i < (size_t) nChannels; ++i)
        delays[i].setDelay (delayRelativeMults[i] * diffusionTimeMs * (FloatType) 0.001 * fs);
}

//======================================================================
template <int nStages, typename DiffuserType>
template <typename DiffuserChainConfig, typename DiffuserConfig>
void DiffuserChain<nStages, DiffuserType>::prepare (double sampleRate)
{
    for (size_t i = 0; i < (size_t) nStages; ++i)
    {
        stages[i].template prepare<DiffuserConfig> (sampleRate);
        diffusionTimeMults[i] = (FloatType) DiffuserChainConfig::getDiffusionMult ((int) i, nStages);
    }
}

template <int nStages, typename DiffuserType>
void DiffuserChain<nStages, DiffuserType>::reset()
{
    for (auto& stage : stages)
        stage.reset();
}

template <int nStages, typename DiffuserType>
void DiffuserChain<nStages, DiffuserType>::setDiffusionTimeMs (FloatType diffusionTimeMs)
{
    for (size_t i = 0; i < (size_t) nStages; ++i)
        stages[i].setDiffusionTimeMs (diffusionTimeMs * diffusionTimeMults[i]);
}
} // namespace chowdsp::Reverb
