#pragma once

namespace chowdsp::Reverb::Dattorro
{
/** Configuration for the Dattorro input network */
template <typename FloatType = float>
struct DefaultInputNetworkConfig
{
    using Float = FloatType;

    static constexpr int inputDiffusionStageLatticeLength = 8192;

    enum class InputDiffusionStage
    {
        Stage1_Part1 = 0,
        Stage1_Part2,
        Stage2_Part1,
        Stage2_Part2,
        NumStages,
    };

    static int getInputDiffusionTimeSamples (InputDiffusionStage diffusionStage, Float fs)
    {
        if (diffusionStage == InputDiffusionStage::Stage1_Part1)
            return juce::roundToInt ((Float) 0.004771 * fs);
        if (diffusionStage == InputDiffusionStage::Stage1_Part2)
            return juce::roundToInt ((Float) 0.003595 * fs);
        if (diffusionStage == InputDiffusionStage::Stage2_Part1)
            return juce::roundToInt ((Float) 0.012735 * fs);
        if (diffusionStage == InputDiffusionStage::Stage2_Part2)
            return juce::roundToInt ((Float) 0.009307 * fs);

        return 1;
    }

    template <typename DInputNetwork>
    static void setInputDiffusionParameters (DInputNetwork& network, FloatType diff1, FloatType diff2)
    {
        network.getStage (InputDiffusionStage::Stage1_Part1).g = diff1;
        network.getStage (InputDiffusionStage::Stage1_Part2).g = diff1;
        network.getStage (InputDiffusionStage::Stage2_Part1).g = diff2;
        network.getStage (InputDiffusionStage::Stage2_Part2).g = diff2;
    }
};

/** Input diffusion network for the Dattorro reverb architecture */
template <typename Config = DefaultInputNetworkConfig<>>
class InputNetwork
{
    using FloatType = typename Config::Float;

public:
    InputNetwork() = default;

    void prepare (FloatType sampleRate)
    {
        for (auto [i, stage] : enumerate (stages))
        {
            stage.reset();
            stage.setDelayLength (Config::getInputDiffusionTimeSamples (static_cast<typename Config::InputDiffusionStage> (i), sampleRate));
        }
    }

    auto& getStage (typename Config::InputDiffusionStage stage)
    {
        return stages[static_cast<size_t> (stage)];
    }

    void reset()
    {
        for (auto& stage : stages)
        {
            stage.reset();
        }
    }

    inline FloatType processSample (FloatType x) noexcept
    {
        for (auto& stage : stages)
            x = stage.process (x);

        return x;
    }

private:
    std::array<Lattice<FloatType, Config::inputDiffusionStageLatticeLength>, (size_t) Config::InputDiffusionStage::NumStages> stages;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputNetwork)
};
} // namespace chowdsp::Reverb::Dattorro
