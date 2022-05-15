#pragma once

namespace chowdsp::Reverb
{
/**
 * Default configuration for a diffuser.
 * To use a custom configuration, make a derived class from this one,
 * and override some methods.
 */
struct DefaultDiffuserConfig
{
    /** Chooses random delay multipliers in equally spaced regions */
    static double getDelayMult (int channelIndex, int nChannels);

    /** Chooses polarity multipliers randomly */
    static double getPolarityMultiplier (int channelIndex, int nChannels);

    static void fillChannelSwapIndexes (size_t* indexes, int numChannels);
};

/**
 * Simple diffuser configuration with:
 *   - Delay diffusion
 *   - Polarity flipping
 *   - Hadamard mixing
 */
template <typename FloatType, int nChannels, typename DelayInterpType = chowdsp::DelayLineInterpolationTypes::None>
class Diffuser
{
    struct DelayType : public chowdsp::DelayLine<FloatType, DelayInterpType>
    {
        DelayType() : chowdsp::DelayLine<FloatType, DelayInterpType> (1 << 18) {}
    };

public:
    using Float = FloatType;

    Diffuser() = default;

    /** Prepares the diffuser for a given sample rate and configuration */
    template <typename DiffuserConfig = DefaultDiffuserConfig>
    void prepare (double sampleRate);

    /** Resets the diffuser state */
    void reset();

    /** Sets the diffusion time in milliseconds */
    void setDiffusionTimeMs (FloatType diffusionTimeMs);

    /** Processes a set of channels */
    inline const FloatType* process (const FloatType* data) noexcept
    {
        // Delay
        for (size_t i = 0; i < (size_t) nChannels; ++i)
        {
            delays[i].pushSample (0, data[i]);
            outData[i] = delays[channelSwapIndexes[i]].popSample (0);
        }

        // Mix with a Hadamard matrix
        chowdsp::MatrixOps::Hadamard<FloatType, nChannels>::inPlace (outData.data());

        // Flip some polarities
        for (size_t i = 0; i < (size_t) nChannels; ++i)
            outData[i] *= polarityMultipliers[i];

        return outData.data();
    }

private:
    std::array<DelayType, (size_t) nChannels> delays;
    std::array<FloatType, (size_t) nChannels> delayRelativeMults;
    std::array<FloatType, (size_t) nChannels> polarityMultipliers;
    std::array<size_t, (size_t) nChannels> channelSwapIndexes;

    alignas (xsimd::default_arch::alignment()) std::array<FloatType, (size_t) nChannels> outData;

    FloatType fs = (FloatType) 48000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Diffuser)
};

/** Configuration of diffusers with equal diffusion times */
struct DiffuserChainEqualConfig
{
    static double getDiffusionMult (int /*stageIndex*/, int /*nStages*/) { return 1.0; }
};

/** Configuration of diffusers where each diffuser is half the length of the next one */
struct DiffuserChainHalfConfig
{
    static double getDiffusionMult (int stageIndex, int nStages)
    {
        return std::pow (2.0, (double) stageIndex - (double) nStages / 2.0 - 0.5);
    }
};

/** A chain of diffusers */
template <int nStages, typename DiffuserType = Diffuser<float, 8>>
class DiffuserChain
{
    using FloatType = typename DiffuserType::Float;

public:
    DiffuserChain() = default;

    /** Prepares the diffuser chain with a given sample rate and configurations */
    template <typename DiffuserChainConfig = DiffuserChainEqualConfig, typename DiffuserConfig = DefaultDiffuserConfig>
    void prepare (double sampleRate);

    /** Resets the state of each diffuser */
    void reset();

    /** Sets the diffusion time of the diffusion chain */
    void setDiffusionTimeMs (FloatType diffusionTimeMs);

    /** Processes a set of channels */
    inline const FloatType* process (FloatType* data) noexcept
    {
        const FloatType* outData = data;
        for (auto& stage : stages)
            outData = stage.process (outData);

        return outData;
    }

private:
    std::array<DiffuserType, (size_t) nStages> stages;
    std::array<FloatType, (size_t) nStages> diffusionTimeMults;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiffuserChain)
};
} // namespace chowdsp::Reverb

#include "chowdsp_Diffuser.cpp"
