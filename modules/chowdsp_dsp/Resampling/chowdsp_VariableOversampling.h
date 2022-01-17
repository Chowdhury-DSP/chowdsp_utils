#pragma once

namespace chowdsp
{
template <typename FloatType = float>
class VariableOversampling
{
public:
    enum class OSFactor
    {
        OneX,
        TwoX,
        FourX,
        EightX,
        SixteenX,
    };

    enum class OSMode
    {
        MinPhase,
        LinPhase,
    };

    VariableOversampling (juce::AudioProcessorValueTreeState& vts, int numChannels = 2, bool useIntegerLatency = false, const juce::String& paramPrefix = "os");

    static void createParameterLayout (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params,
                                       std::initializer_list<OSFactor> osFactors,
                                       std::initializer_list<OSMode> osModes,
                                       OSFactor defaultFactor,
                                       OSMode defaultMode,
                                       bool includeRenderOptions = true,
                                       const juce::String& paramPrefix = "os");

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void reset();

    int getOSFactor() const noexcept { return (int) overSamplers[(size_t) curOS]->getOversamplingFactor(); }

    bool updateOSFactor();
    float getLatencySamples() const noexcept { return (float) overSamplers[curOS]->getLatencyInSamples(); }
    float getLatencyMilliseconds (int osIndex) const noexcept { return ((float) overSamplers[osIndex]->getLatencyInSamples() / sampleRate) * 1000.0f; }

    auto processSamplesUp (const juce::dsp::AudioBlock<const FloatType>& inputBlock) noexcept { return overSamplers[curOS]->processSamplesUp (inputBlock); }
    void processSamplesDown (juce::dsp::AudioBlock<FloatType>& outputBlock) noexcept { overSamplers[curOS]->processSamplesDown (outputBlock); }

private:
    juce::AudioParameterChoice* osParam = nullptr;
    juce::AudioParameterChoice* osModeParam = nullptr;
    juce::AudioParameterChoice* osOfflineParam = nullptr;
    juce::AudioParameterChoice* osOfflineModeParam = nullptr;
    juce::AudioParameterChoice* osOfflineSameParam = nullptr;

    int curOS = 0, prevOS = 0;
    int numOSChoices = 0;
    float sampleRate = 48000.0f;

    std::vector<std::unique_ptr<juce::dsp::Oversampling<FloatType>>> overSamplers;

    const juce::AudioProcessor& proc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VariableOversampling)
};
} // namespace chowdsp
