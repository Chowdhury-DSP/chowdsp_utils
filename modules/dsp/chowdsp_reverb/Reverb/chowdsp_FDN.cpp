#include "chowdsp_FDN.h"

namespace chowdsp::Reverb
{
template <typename FloatType, int nChannels, typename StorageType>
void DefaultFDNConfig<FloatType, nChannels, StorageType>::prepare (double sampleRate)
{
    fs = (FloatType) sampleRate;
}

template <typename FloatType, int nChannels, typename StorageType>
void DefaultFDNConfig<FloatType, nChannels, StorageType>::reset()
{
    for (auto& filt : shelfs)
        filt.reset();
}

template <typename FloatType, int nChannels, typename StorageType>
double DefaultFDNConfig<FloatType, nChannels, StorageType>::getDelayMult (int channelIndex)
{
    std::random_device rd;
    std::mt19937 randGenerator (rd());
    return DefaultDiffuserConfig::getDelayMult (channelIndex, nChannels, randGenerator);
}

template <typename FloatType, int nChannels, typename StorageType>
void DefaultFDNConfig<FloatType, nChannels, StorageType>::applyMixingMatrix (FloatType* data)
{
    MatrixOps::HouseHolder<FloatType, nChannels>::inPlace (data);
}

template <typename FloatType, int nChannels, typename StorageType>
FloatType DefaultFDNConfig<FloatType, nChannels, StorageType>::calcGainForT60 (FloatType decayTimeMs, FloatType delayTimeMs)
{
    const auto nTimes = decayTimeMs / delayTimeMs;
    return std::pow ((FloatType) 0.001, (FloatType) 1 / nTimes);
}

template <typename FloatType, int nChannels, typename StorageType>
template <typename FDNType>
void DefaultFDNConfig<FloatType, nChannels, StorageType>::setDecayTimeMs (const FDNType& fdn, FloatType decayTimeLowMs, FloatType decayTimeHighMs, FloatType crossoverFreqHz)
{
    for (size_t i = 0; i < (size_t) nChannels; ++i)
    {
        const auto channelDelayMs = fdn.getChannelDelayMs (i);
        const auto lowGain = calcGainForT60 (decayTimeLowMs, channelDelayMs);
        const auto highGain = calcGainForT60 (decayTimeHighMs, channelDelayMs);
        shelfs[i].calcCoefs (lowGain, highGain, crossoverFreqHz, fs);
    }
}

template <typename FloatType, int nChannels, typename StorageType>
const FloatType* DefaultFDNConfig<FloatType, nChannels, StorageType>::doFeedbackProcess (DefaultFDNConfig& fdnConfig, const FloatType* data)
{
    for (size_t i = 0; i < (size_t) nChannels; ++i)
        fdnConfig.fbData[i] = fdnConfig.shelfs[i].processSample (data[i]);

    return fdnConfig.fbData.data();
}

//======================================================================
template <typename FDNConfig, typename DelayInterpType, int delayBufferSize>
void FDN<FDNConfig, DelayInterpType, delayBufferSize>::prepare (double sampleRate)
{
    fsOver1000 = (FloatType) sampleRate / (FloatType) 1000;
    for (size_t i = 0; i < (size_t) nChannels; ++i)
    {
        delays[i].reset();
        delayWritePointer = 0;

        delayRelativeMults[i] = (FloatType) FDNConfig::getDelayMult ((int) i);
    }

    fdnConfig.prepare (sampleRate);
}

template <typename FDNConfig, typename DelayInterpType, int delayBufferSize>
void FDN<FDNConfig, DelayInterpType, delayBufferSize>::reset()
{
    fdnConfig.reset();
    for (auto& delay : delays)
        delay.reset();
}

template <typename FDNConfig, typename DelayInterpType, int delayBufferSize>
void FDN<FDNConfig, DelayInterpType, delayBufferSize>::setDelayTimeMs (FloatType delayTimeMs)
{
    for (size_t i = 0; i < (size_t) nChannels; ++i)
    {
        delayTimesSamples[i] = delayRelativeMults[i] * delayTimeMs * fsOver1000;
        delayReadPointers[i] = DelayType::getReadPointer (delayWritePointer, delayTimesSamples[i]);
    }
}

template <typename FDNConfig, typename DelayInterpType, int delayBufferSize>
template <int NModulators>
void FDN<FDNConfig, DelayInterpType, delayBufferSize>::setDelayTimeMsWithModulators (FloatType delayTimeMs, FloatType (&modulationAmt)[(size_t) NModulators])
{
    static_assert (NModulators <= nChannels, "Can't have more modulators than channels!");

    for (size_t i = 0; i < (size_t) NModulators; ++i)
    {
        delayTimesSamples[i] = delayRelativeMults[i] * delayTimeMs * fsOver1000 * modulationAmt[i];
        delayReadPointers[i] = DelayType::getReadPointer (delayWritePointer, delayTimesSamples[i]);
    }

    for (auto i = (size_t) NModulators; i < (size_t) nChannels; ++i)
    {
        delayTimesSamples[i] = delayRelativeMults[i] * delayTimeMs * fsOver1000;
        delayReadPointers[i] = DelayType::getReadPointer (delayWritePointer, delayTimesSamples[i]);
    }
}

template <typename FDNConfig, typename DelayInterpType, int delayBufferSize>
typename FDN<FDNConfig, DelayInterpType, delayBufferSize>::FloatType FDN<FDNConfig, DelayInterpType, delayBufferSize>::getChannelDelayMs (size_t channelIndex) const noexcept
{
    return delayTimesSamples[channelIndex] / fsOver1000;
}
} // namespace chowdsp::Reverb
