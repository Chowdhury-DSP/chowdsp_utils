#pragma once
/*
namespace chowdsp::Reverb
{
template <typename FloatType>
struct DefaultDattorroConfig
{
    using Float = FloatType;

    static constexpr int decayDiffusionStageLatticeLength = 1 << 15;
    static constexpr int tankDelayMax = 1 << 16;

    enum class DecayDiffusion1
    {
        Left = 0,
        Right,
        NumStages,
    };

    static int getDecayDiffusion1TimeSamples (DecayDiffusion1 diffusionStage, Float fs)
    {
        if (diffusionStage == DecayDiffusion1::Left)
            return juce::roundToInt ((Float) 0.022579886 * fs);
        if (diffusionStage == DecayDiffusion1::Right)
            return juce::roundToInt ((Float) 0.030509727 * fs);
        return 1;
    }

    template <typename DReverb>
    static void setDecayDiffusion1Parameters (DReverb& reverb, FloatType diff)
    {
        reverb.getDecayDiffusion1Stage (DecayDiffusion1::Left).g = diff;
        reverb.getDecayDiffusion1Stage (DecayDiffusion1::Right).g = diff;
    }

    enum class DecayDiffusion2
    {
        Left = 0,
        Right,
        NumStages,
    };

    static int getDecayDiffusion2TimeSamples (DecayDiffusion2 diffusionStage, Float fs)
    {
        if (diffusionStage == DecayDiffusion2::Left)
            return juce::roundToInt ((Float) 0.0604818 * fs);
        if (diffusionStage == DecayDiffusion2::Right)
            return juce::roundToInt ((Float) 0.0892443 * fs);
        return 1;
    }

    template <typename DReverb>
    static void setDecayDiffusion2Parameters (DReverb& reverb, FloatType diff)
    {
        reverb.getDecayDiffusion2Stage (DecayDiffusion2::Left).g = diff;
        reverb.getDecayDiffusion2Stage (DecayDiffusion2::Right).g = diff;
    }

    static int getTankDelay1TimeSamples (DecayDiffusion1 diffusionStage, Float fs)
    {
        if (diffusionStage == DecayDiffusion1::Left)
            return juce::roundToInt ((Float) 0.1496253 * fs);
        if (diffusionStage == DecayDiffusion1::Right)
            return juce::roundToInt ((Float) 0.1416955 * fs);
        return 1;
    }

    static int getTankDelay2TimeSamples (DecayDiffusion2 diffusionStage, Float fs)
    {
        if (diffusionStage == DecayDiffusion2::Left)
            return juce::roundToInt ((Float) 0.1249958 * fs);
        if (diffusionStage == DecayDiffusion2::Right)
            return juce::roundToInt ((Float) 0.10628003 * fs);
        return 1;
    }
};

template <typename Config>
class DattorroReverb
{
    using FloatType = typename Config::Float;

    template <int maxSize>
    using DelayType = chowdsp::StaticDelayBuffer<FloatType, DelayLineInterpolationTypes::None, maxSize>;

public:


    DattorroReverb() = default;

    void prepare (FloatType sampleRate)
    {
        for (auto [i, stage] : enumerate (inputStages))
        {
            stage.reset();
            stage.setDelayLength (Config::getInputDiffusionTimeSamples (static_cast<typename Config::InputDiffusionStage> (i), sampleRate));
        }

        for (auto [i, stage] : enumerate (decayStages1))
        {
            stage.reset();
            stage.setDelayLength (Config::getDecayDiffusion1TimeSamples (static_cast<typename Config::DecayDiffusion1> (i), sampleRate));
        }

        for (auto [i, stage] : enumerate (decayStages2))
        {
            stage.reset();
            stage.setDelayLength (Config::getDecayDiffusion2TimeSamples (static_cast<typename Config::DecayDiffusion2> (i), sampleRate));
        }

        writePointer = 0;
        for (auto [i, delay] : enumerate (diffusion1Delay))
        {
            delay.reset();
            const auto delaySamples = Config::getTankDelay1TimeSamples (static_cast<typename Config::DecayDiffusion1> (i), sampleRate);
            readPointer[i] = TankDelay::getReadPointer (writePointer, delaySamples);
        }

        for (auto [i, delay] : enumerate (diffusion2Delay))
        {
            delay.reset();
            const auto delaySamples = Config::getTankDelay2TimeSamples (static_cast<typename Config::DecayDiffusion2> (i), sampleRate);
            readPointer[i + numChannels] = TankDelay::getReadPointer (writePointer, delaySamples);
        }
    }

    auto& getInputDiffusionStage (typename Config::InputDiffusionStage stage)
    {
        return inputStages[static_cast<size_t> (stage)];
    }

    auto& getDecayDiffusion1Stage (typename Config::DecayDiffusion1 stage)
    {
        return decayStages1[static_cast<size_t> (stage)];
    }

    auto& getDecayDiffusion2Stage (typename Config::DecayDiffusion2 stage)
    {
        return decayStages2[static_cast<size_t> (stage)];
    }

    void setDecayAmount (FloatType decay)
    {
        decayMult = decay;
        Config::setDecayDiffusion2Parameters (*this, juce::jlimit ((FloatType) 0.25, FloatType (0.5), decay + (FloatType) 0.15));
    }

    inline FloatType processSample (FloatType x) noexcept
    {
        for (auto& stage : inputStages)
            x = stage.process (x);

        for (size_t i = 0; i < numChannels; ++i)
        {
            // decay stage 1
            tankValues[i] = decayStages1[i].process (x + tankValues[i] * decayMult);

            // delay stage 1
            diffusion1Delay[i].pushSample (tankValues[i], writePointer);
            tankValues[i] = diffusion1Delay[i].popSample ((FloatType) readPointer[i]);
            TankDelay::decrementPointer (readPointer[i]);

            // @TODO: damping
            tankValues[i] *= decayMult;

            // decay stage 2
            tankValues[i] = decayStages2[i].process (tankValues[i]);

            // delay stage 2
            diffusion2Delay[i].pushSample (tankValues[i], writePointer);
            tankValues[i] = diffusion2Delay[i].popSample ((FloatType) readPointer[numChannels + i]);
            TankDelay::decrementPointer (readPointer[numChannels + i]);
        }

        TankDelay::decrementPointer (writePointer);

        return std::accumulate (std::begin (tankValues), std::end (tankValues), FloatType {}) * ((FloatType) 1 / (FloatType) numChannels);
    }

private:
    std::array<Lattice<Config::inputDiffusionStageLatticeLength>, (size_t) Config::InputDiffusionStage::NumStages> inputStages;
    std::array<Lattice<Config::decayDiffusionStageLatticeLength, true>, (size_t) Config::DecayDiffusion1::NumStages> decayStages1;
    std::array<Lattice<Config::decayDiffusionStageLatticeLength>, (size_t) Config::DecayDiffusion2::NumStages> decayStages2;

    static constexpr auto numChannels = (size_t) Config::DecayDiffusion1::NumStages;
    FloatType tankValues[numChannels] {};

    using TankDelay = StaticDelayBuffer<FloatType, DelayLineInterpolationTypes::None, Config::tankDelayMax>;
    TankDelay diffusion1Delay[numChannels];
    TankDelay diffusion2Delay[numChannels];
    int writePointer = 0;
    int readPointer[2 * numChannels] {};

    FloatType decayMult {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DattorroReverb)
};
} // namespace chowdsp::Reverb
*/