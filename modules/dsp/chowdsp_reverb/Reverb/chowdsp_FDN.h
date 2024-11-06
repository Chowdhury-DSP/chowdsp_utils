#pragma once

namespace chowdsp::Reverb
{
/** Default configuration for a feedback delay network */
template <typename FloatType, int nChannels, typename StorageType = FloatType>
struct DefaultFDNConfig
{
    using Float = FloatType;
    using FloatStorageType = StorageType;
    static constexpr auto NChannels = nChannels;

    /** Prepares any internal processors */
    virtual void prepare (double sampleRate);

    /** Resets the state of the internal processors */
    virtual void reset();

    /** Chooses random delay multipliers in equally spaced regions */
    static double getDelayMult (int channelIndex);

    /** Applies a mixing matrix to the given data (in place) */
    static void applyMixingMatrix (FloatType* data);

    /** Returns the gain needed for a given decay time (T60), given the delay time */
    static FloatType calcGainForT60 (FloatType decayTimeMs, FloatType delayTimeMs);

    /** Sets the decay time (T60) for high and low frequencies, as well as a transition crossover frequency */
    template <typename FDNType>
    void setDecayTimeMs (const FDNType& fdn, FloatType decayTimeLowMs, FloatType decayTimeHighMs, FloatType crossoverFreqHz);

    /** Implements the feedback processing through and internal processors */
    static const FloatType* doFeedbackProcess (DefaultFDNConfig& fdnConfig, const FloatType* data);

protected:
#if CHOWDSP_REVERB_ALIGN_IO
    alignas (SIMDUtils::defaultSIMDAlignment) std::array<FloatType, (size_t) nChannels> fbData;
#else
    std::array<FloatType, (size_t) nChannels> fbData;
#endif

private:
    std::array<ShelfFilter<FloatType>, (size_t) nChannels> shelfs;
    FloatType fs = 48000.0f;
};

/** A feedback delay network processor with a customizable configuration */
template <typename FDNConfig, typename DelayInterpType = DelayLineInterpolationTypes::None, int delayBufferSize = 1 << 18>
class FDN
{
    static_assert (types_list::TypesList<DelayLineInterpolationTypes::None,
                                         DelayLineInterpolationTypes::Linear>::
                       contains<DelayInterpType>,
                   "DelayInterpType must be either None or Linear");

    using FloatType = typename FDNConfig::Float;
    static constexpr auto nChannels = FDNConfig::NChannels;

    using DelayType = StaticDelayBuffer<FloatType, DelayInterpType, delayBufferSize, typename FDNConfig::FloatStorageType>;

public:
    FDN() = default;

    /** Prepares the FDN for a given sample rate */
    void prepare (double sampleRate);

    /** Resets the FDN state */
    void reset();

    /** Sets the max delay time in milliseconds */
    void setDelayTimeMs (FloatType delayTimeMs);

    /**
     * Sets the max delay time in milliseconds, with modulation applied to the first N modulators.
     * Note that modulation is applied as a multiplier of the base delay time.
     */
    template <int NModulators>
    void setDelayTimeMsWithModulators (FloatType delayTimeMs, FloatType (&modulationAmt)[(size_t) NModulators]);

    /** Returns the delay time of a given FDN channel */
    FloatType getChannelDelayMs (size_t channelIndex) const noexcept;

    /** Processes a set of channels */
    inline const FloatType* process (const FloatType* data) noexcept
    {
        // read from delay lines
        for (size_t i = 0; i < (size_t) nChannels; ++i)
        {
            outData[i] = delays[i].popSample (delayReadPointers[i]);
            DelayType::decrementPointer (delayReadPointers[i]);
        }

        // do mixing matrix
        FDNConfig::applyMixingMatrix (outData.data());

        // do other feedback processing
        const auto* fbData = FDNConfig::doFeedbackProcess (fdnConfig, outData.data());

        // write back to delay lines
        for (size_t i = 0; i < (size_t) nChannels; ++i)
            delays[i].pushSample (data[i] + fbData[i], delayWritePointer);
        DelayType::decrementPointer (delayWritePointer);

        return outData.data();
    }

    /** Returns the FDN configuration object */
    auto& getFDNConfig() { return fdnConfig; }

private:
    std::array<DelayType, (size_t) nChannels> delays;
    std::array<FloatType, (size_t) nChannels> delayRelativeMults;

    std::array<FloatType, (size_t) nChannels> delayTimesSamples;
    int delayWritePointer = 0;
    std::array<FloatType, (size_t) nChannels> delayReadPointers;

    FDNConfig fdnConfig;

#if CHOWDSP_REVERB_ALIGN_IO
    alignas (SIMDUtils::defaultSIMDAlignment) std::array<FloatType, (size_t) nChannels> outData;
#else
    std::array<FloatType, (size_t) nChannels> outData;
#endif

    FloatType fsOver1000 = FloatType (48000 / 1000);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FDN)
};
} // namespace chowdsp::Reverb

#include "chowdsp_FDN.cpp"
