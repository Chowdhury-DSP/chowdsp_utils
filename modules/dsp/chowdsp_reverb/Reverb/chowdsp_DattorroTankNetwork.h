#pragma once

namespace chowdsp::Reverb::Dattorro
{
template <typename FloatType = float>
struct DefaultTankNetworkConfig
{
    using Float = FloatType;

    static constexpr int decayDiffusionStageLatticeLength = 1 << 15;
    static constexpr int tankDelayMax = 1 << 16;

    static int getDecayDiffusion1TimeSamples (size_t diffusionStage, Float fs)
    {
        if (diffusionStage == 0)
            return juce::roundToInt ((Float) 0.022579886 * fs);
        if (diffusionStage == 1)
            return juce::roundToInt ((Float) 0.030509727 * fs);
        return 1;
    }

    template <typename DReverb>
    static void setDecayDiffusion1Parameters (DReverb& reverb, FloatType diff)
    {
        reverb.getDecayDiffusion1Stage (0).g = diff;
        reverb.getDecayDiffusion1Stage (1).g = diff;
    }

    static int getDecayDiffusion2TimeSamples (size_t diffusionStage, Float fs)
    {
        if (diffusionStage == 0)
            return juce::roundToInt ((Float) 0.0604818 * fs);
        if (diffusionStage == 1)
            return juce::roundToInt ((Float) 0.0892443 * fs);
        return 1;
    }

    template <typename DReverb>
    static void setDecayDiffusion2Parameters (DReverb& reverb, FloatType diff)
    {
        reverb.getDecayDiffusion2Stage (0).g = diff;
        reverb.getDecayDiffusion2Stage (1).g = diff;
    }

    static int getTankDelay1TimeSamples (size_t diffusionStage, Float fs)
    {
        if (diffusionStage == 0)
            return juce::roundToInt ((Float) 0.1496253 * fs);
        if (diffusionStage == 1)
            return juce::roundToInt ((Float) 0.1416955 * fs);
        return 1;
    }

    static int getTankDelay2TimeSamples (size_t diffusionStage, Float fs)
    {
        if (diffusionStage == 0)
            return juce::roundToInt ((Float) 0.1249958 * fs);
        if (diffusionStage == 1)
            return juce::roundToInt ((Float) 0.10628003 * fs);
        return 1;
    }

    static int getOutputTapSample (size_t channel, size_t tapIndex, Float fs)
    {
        if (channel == 0)
        {
            if (tapIndex == 0)
                return juce::roundToInt ((Float) 0.00893787 * fs);
            if (tapIndex == 1)
                return juce::roundToInt ((Float) 0.09992944 * fs);
            if (tapIndex == 2)
                return juce::roundToInt ((Float) 0.06427875 * fs);
            if (tapIndex == 3)
                return juce::roundToInt ((Float) 0.06706764 * fs);
            if (tapIndex == 4)
                return juce::roundToInt ((Float) 0.06686603 * fs);
            if (tapIndex == 5)
                return juce::roundToInt ((Float) 0.00628339 * fs);
            if (tapIndex == 6)
                return juce::roundToInt ((Float) 0.03581869 * fs);
        }

        if (channel == 1)
        {
            if (tapIndex == 0)
                return juce::roundToInt ((Float) 0.01186116 * fs);
            if (tapIndex == 1)
                return juce::roundToInt ((Float) 0.12187090 * fs);
            if (tapIndex == 2)
                return juce::roundToInt ((Float) 0.04126205 * fs);
            if (tapIndex == 3)
                return juce::roundToInt ((Float) 0.08981553 * fs);
            if (tapIndex == 4)
                return juce::roundToInt ((Float) 0.07093176 * fs);
            if (tapIndex == 5)
                return juce::roundToInt ((Float) 0.01125634 * fs);
            if (tapIndex == 6)
                return juce::roundToInt ((Float) 0.00406572 * fs);
        }

        return 1;
    }
};

/**
 * Diffusion and decay "tank" network for the Dattorro reverb architecture.
 */
template <typename Config = DefaultTankNetworkConfig<>>
class TankNetwork
{
    using FloatType = typename Config::Float;

    template <int maxSize>
    using DelayType = StaticDelayBuffer<FloatType, DelayLineInterpolationTypes::None, maxSize>;

public:
    TankNetwork() = default;

    void prepare (FloatType sampleRate)
    {
        for (auto [i, stage] : enumerate (decayStages1))
        {
            stage.reset();
            stage.setDelayLength (Config::getDecayDiffusion1TimeSamples (i, sampleRate));
        }

        for (auto [i, stage] : enumerate (decayStages2))
        {
            stage.reset();
            stage.setDelayLength (Config::getDecayDiffusion2TimeSamples (i, sampleRate));
        }

        writePointer = 0;
        for (auto [i, delay] : enumerate (diffusion1Delay))
        {
            delay.reset();
            const auto delaySamples = Config::getTankDelay1TimeSamples (i, sampleRate);
            readPointer[i] = TankDelay::getReadPointer (writePointer, delaySamples);
        }

        for (auto [i, delay] : enumerate (diffusion2Delay))
        {
            delay.reset();
            const auto delaySamples = Config::getTankDelay2TimeSamples (i, sampleRate);
            readPointer[i + numChannels] = TankDelay::getReadPointer (writePointer, delaySamples);
        }

        fs = sampleRate;
        for (auto& filt : dampingFilters)
            filt.reset();

        for (size_t i = 0; i < 7; ++i)
        {
            leftOutTaps[i] = Config::getOutputTapSample (0, i, sampleRate);
            rightOutTaps[i] = Config::getOutputTapSample (1, i, sampleRate);
        }
    }

    void reset()
    {
        for (auto& stage : decayStages1)
            stage.reset();

        for (auto& stage : decayStages2)
            stage.reset();

        for (auto& delay : diffusion1Delay)
            delay.reset();

        for (auto& delay : diffusion2Delay)
            delay.reset();

        for (auto& filt : dampingFilters)
            filt.reset();
    }

    auto& getDecayDiffusion1Stage (int stage)
    {
        return decayStages1[static_cast<size_t> (stage)];
    }

    auto& getDecayDiffusion2Stage (int stage)
    {
        return decayStages2[static_cast<size_t> (stage)];
    }

    void setDecayAmount (FloatType decay)
    {
        decayMult = decay;
        Config::setDecayDiffusion2Parameters (*this, juce::jlimit ((FloatType) 0.25, FloatType (0.5), decay + (FloatType) 0.15));
    }

    void setDampingFrequency (FloatType freqHz)
    {
        for (auto& filt : dampingFilters)
            filt.calcCoefs (freqHz, fs);
    }

    inline std::pair<FloatType, FloatType> processSample (FloatType xLeft, FloatType xRight) noexcept
    {
        for (size_t i = 0; i < numChannels; ++i)
        {
            // decay stage 1
            tankValues[i] = decayStages1[i].process ((i == 0 ? xLeft : xRight) + tankValues[i] * decayMult);

            // delay stage 1
            diffusion1Delay[i].pushSample (tankValues[i], writePointer);
            tankValues[i] = diffusion1Delay[i].popSample ((FloatType) readPointer[i]);
            TankDelay::decrementPointer (readPointer[i]);

            // Damping and intermediate decay
            tankValues[i] = dampingFilters[i].processSample (tankValues[i]);
            tankValues[i] *= decayMult;

            // decay stage 2
            tankValues[i] = decayStages2[i].process (tankValues[i]);

            // delay stage 2
            diffusion2Delay[i].pushSample (tankValues[i], writePointer);
            tankValues[i] = diffusion2Delay[i].popSample ((FloatType) readPointer[numChannels + i]);
            TankDelay::decrementPointer (readPointer[numChannels + i]);
        }

        FloatType yLeft {};
        yLeft += tapDelayLine (diffusion1Delay[1], leftOutTaps[0], writePointer);
        yLeft += tapDelayLine (diffusion1Delay[1], leftOutTaps[1], writePointer);
        yLeft -= decayStages2[1].getTap (leftOutTaps[2]);
        yLeft += tapDelayLine (diffusion2Delay[1], leftOutTaps[3], writePointer);
        yLeft -= tapDelayLine (diffusion1Delay[0], leftOutTaps[4], writePointer);
        yLeft -= decayStages2[0].getTap (leftOutTaps[5]);
        yLeft -= tapDelayLine (diffusion2Delay[0], leftOutTaps[6], writePointer);
        yLeft *= (FloatType) 0.6;

        FloatType yRight {};
        yRight += tapDelayLine (diffusion1Delay[0], rightOutTaps[0], writePointer);
        yRight += tapDelayLine (diffusion1Delay[0], rightOutTaps[1], writePointer);
        yRight -= decayStages2[0].getTap (rightOutTaps[2]);
        yRight += tapDelayLine (diffusion2Delay[0], rightOutTaps[3], writePointer);
        yRight -= tapDelayLine (diffusion1Delay[1], rightOutTaps[4], writePointer);
        yRight -= decayStages2[1].getTap (rightOutTaps[5]);
        yRight -= tapDelayLine (diffusion2Delay[1], rightOutTaps[6], writePointer);
        yRight *= (FloatType) 0.6;

        TankDelay::decrementPointer (writePointer);
        return std::make_pair (yLeft, yRight);
    }

private:
    using TankDelay = DelayType<Config::tankDelayMax>;
    static FloatType tapDelayLine (TankDelay& delay, int tapSample, int wp) noexcept
    {
        const auto rp = TankDelay::getReadPointer (wp, tapSample);
        return delay.popSample ((FloatType) rp);
    }

    static constexpr size_t numChannels = 2;
    std::array<Lattice<FloatType, Config::decayDiffusionStageLatticeLength, true>, numChannels> decayStages1;
    std::array<Lattice<FloatType, Config::decayDiffusionStageLatticeLength>, numChannels> decayStages2;

    FloatType tankValues[numChannels] {};

    TankDelay diffusion1Delay[numChannels];
    TankDelay diffusion2Delay[numChannels];
    int writePointer = 0;
    int readPointer[2 * numChannels] {};

    FloatType decayMult {};

    std::array<FirstOrderLPF<FloatType>, numChannels> dampingFilters;
    FloatType fs = (FloatType) 48000.0;

    int leftOutTaps[7] {};
    int rightOutTaps[7] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TankNetwork)
};
} // namespace chowdsp::Reverb::Dattorro
