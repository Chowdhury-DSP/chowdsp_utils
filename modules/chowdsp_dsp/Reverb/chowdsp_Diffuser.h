#pragma once

namespace chowdsp::Reverb
{

struct DefaultDiffuserConfig
{
    virtual double getDelaySamples (int channelIndex, int nChannels, double sampleRate);

    virtual double getPolarityMultiplier (int channelIndex, int nChannels);

    double delayRangeMs = 300.0;
    juce::Random rand;
};

template <typename FloatType, int nChannels, typename DelayInterpType = chowdsp::DelayLineInterpolationTypes::None>
class Diffuser
{
    using NumericType = typename SampleTypeHelpers::ElementType<FloatType>::Type;
    using DelayType = chowdsp::DelayLine<FloatType, DelayInterpType>;

public:
    using Float = FloatType;

    Diffuser();

    template <typename DiffuserConfig = DefaultDiffuserConfig>
    void prepare (double sampleRate, const DiffuserConfig& diffuserConfig = DefaultDiffuserConfig {});

    inline void process (FloatType* data) noexcept
    {
        // Delay
        for (int i = 0; i < nChannels; ++i)
        {
            delays[i].pushSample (0, data[i]);
            data[i] = data[i].popSample (0);
        }

        // Mix with a Hadamard matrix
        MatrixOps::Hadamard<FloatType, nChannels>::inPlace (data);

        // Flip some polarities
        for (int i = 0; i < nChannels; ++i)
            data[i] *= polarityMultipliers[i];
    }

private:
    std::array<DelayType, nChannels> delays;
    std::array<FloatType, nChannels> polarityMultipliers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Diffuser)
};

template <int nStages, typename DiffuserType = Diffuser<float, 8>>
class DiffuserChain
{
public:
    DiffuserChain() = default;

    void prepare (double sampleRate)
    {
        for (auto& stage : stages)
            stage.prepare (sampleRate);
    }

    inline void process (typename DiffuserType::Float* data) noexcept
    {
        for (auto& stage : stages)
            stage.process (data);
    }

private:
    std::array<DiffuserType, nStages> stages;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiffuserChain)
};

} // namespace chowdsp::Reverb

#include "chowdsp_Diffuser.cpp"
