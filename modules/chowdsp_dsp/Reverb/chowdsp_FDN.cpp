#include "chowdsp_FDN.h"

namespace chowdsp::Reverb
{
template <typename FloatType, int nChannels>
void DefaultFDNConfig<FloatType, nChannels>::prepare (double sampleRate)
{
    fs = (FloatType) sampleRate;
}

template <typename FloatType, int nChannels>
void DefaultFDNConfig<FloatType, nChannels>::reset()
{
    for (auto& filt : shelfs)
        filt.reset();
}

template <typename FloatType, int nChannels>
double DefaultFDNConfig<FloatType, nChannels>::getDelayMult (int channelIndex)
{
    return DefaultDiffuserConfig::getDelayMult (channelIndex, nChannels);
}

template <typename FloatType, int nChannels>
void DefaultFDNConfig<FloatType, nChannels>::applyMixingMatrix (FloatType* data)
{
    chowdsp::MatrixOps::HouseHolder<FloatType, nChannels>::inPlace (data);
}

template <typename FloatType, int nChannels>
FloatType DefaultFDNConfig<FloatType, nChannels>::calcGainForT60 (FloatType decayTimeMs, FloatType delayTimeMs)
{
    const auto nTimes = decayTimeMs / delayTimeMs;
    return std::pow ((FloatType) 0.001, (FloatType) 1 / nTimes);
}

template <typename FloatType, int nChannels>
template <typename FDNType>
void DefaultFDNConfig<FloatType, nChannels>::setDecayTimeMs (const FDNType& fdn, FloatType decayTimeLowMs, FloatType decayTimeHighMs, FloatType crossoverFreqHz)
{
    for (size_t i = 0; i < (size_t) nChannels; ++i)
    {
        const auto channelDelayMs = fdn.getChannelDelayMs (i);
        const auto lowGain = calcGainForT60 (decayTimeLowMs, channelDelayMs);
        const auto highGain = calcGainForT60 (decayTimeHighMs, channelDelayMs);
        shelfs[i].calcCoefs (lowGain, highGain, crossoverFreqHz, fs);
    }
}

template <typename FloatType, int nChannels>
const FloatType* DefaultFDNConfig<FloatType, nChannels>::doFeedbackProcess (DefaultFDNConfig& fdnConfig, const FloatType* data)
{
    for (size_t i = 0; i < (size_t) nChannels; ++i)
        fdnConfig.fbData[i] = fdnConfig.shelfs[i].processSample (data[i]);

    return fdnConfig.fbData.data();
}

//======================================================================
template <typename FDNConfig, typename DelayInterpType>
void FDN<FDNConfig, DelayInterpType>::prepare (double sampleRate)
{
    fs = (FloatType) sampleRate;
    for (size_t i = 0; i < (size_t) nChannels; ++i)
    {
        delays[i].prepare ({ sampleRate, 128, 1 });
        delayRelativeMults[i] = (FloatType) FDNConfig::getDelayMult ((int) i);
    }

    fdnConfig.prepare (sampleRate);
}

template <typename FDNConfig, typename DelayInterpType>
void FDN<FDNConfig, DelayInterpType>::reset()
{
    fdnConfig.reset();
    for (auto& delay : delays)
        delay.reset();
}

template <typename FDNConfig, typename DelayInterpType>
void FDN<FDNConfig, DelayInterpType>::setDelayTimeMs (FloatType delayTimeMs)
{
    for (size_t i = 0; i < (size_t) nChannels; ++i)
        delays[i].setDelay (delayRelativeMults[i] * delayTimeMs * (FloatType) 0.001 * fs);
}

template <typename FDNConfig, typename DelayInterpType>
template <int NModulators>
void FDN<FDNConfig, DelayInterpType>::setDelayTimeMsWithModulators (FloatType delayTimeMs, FloatType (&modulationAmt)[(size_t) NModulators])
{
    static_assert (NModulators <= nChannels, "Can't have more modulators than channels!");

    for (size_t i = 0; i < (size_t) NModulators; ++i)
        delays[i].setDelay (delayRelativeMults[i] * delayTimeMs * (FloatType) 0.001 * fs * modulationAmt[i]);

    for (auto i = (size_t) NModulators; i < (size_t) nChannels; ++i)
        delays[i].setDelay (delayRelativeMults[i] * delayTimeMs * (FloatType) 0.001 * fs);
}

template <typename FDNConfig, typename DelayInterpType>
typename FDN<FDNConfig, DelayInterpType>::FloatType FDN<FDNConfig, DelayInterpType>::getChannelDelayMs (size_t channelIndex) const noexcept
{
    return delays[channelIndex].getDelay() / (fs * (FloatType) 0.001);
}
} // namespace chowdsp::Reverb
