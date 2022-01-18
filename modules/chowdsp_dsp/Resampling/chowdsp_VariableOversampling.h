#pragma once

namespace chowdsp
{
/**
 * Let's say you've got a plugin that should have oversampling options.
 * This class can be used to create a set of oversamplers connected to
 * parameters that manage the oversampling factor and mode, and may also
 * be used to manage a separate oversampling mode for offline renderring.
 */
template <typename FloatType = float>
class VariableOversampling
{
public:
    /** Enum describing the different oversampling factors that are supported */
    enum class OSFactor
    {
        OneX,
        TwoX,
        FourX,
        EightX,
        SixteenX,
    };

    /** Enum describing the different oversampling modes that are supported */
    enum class OSMode
    {
        MinPhase,
        LinPhase,
    };

    /**
     * Construct a variable oversampling object with an APVTS
     *
     * @param vts               APVTS that has been initialized with the parameters from `createParameterLayout()`
     * @param numChannels       Number of channels that may be processed (either 1 or 2)
     * @param useIntegerLatency Set to true to force the oversamplers to use integer latency
     * @param paramPrefix       The same parameter prefix used to create the parameters
     */
    explicit VariableOversampling (juce::AudioProcessorValueTreeState& vts, int numChannels = 2, bool useIntegerLatency = false, const juce::String& paramPrefix = "os");

    /**
     * Creates a parameter layout for variable oversampling,
     * with all available oversampling options.
     *
     * @param params                Vector of parameters to add to
     * @param defaultFactor         Default oversampling factor
     * @param defaultMode           Default oversamplign mode
     * @param includeRenderOptions  Set to true to include parameters for oversampling when doing offline renderring
     * @param paramPrefix           Parameter prefix to use for parameter IDs
     */
    static void createParameterLayout (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params,
                                       OSFactor defaultFactor,
                                       OSMode defaultMode,
                                       bool includeRenderOptions = true,
                                       const juce::String& paramPrefix = "os");

    /**
     * Creates a parameter layout for variable oversampling,
     * with custom choices for which oversampling factors and modes
     * may be used.
     */
    static void createParameterLayout (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params,
                                       std::initializer_list<OSFactor> osFactors,
                                       std::initializer_list<OSMode> osModes,
                                       OSFactor defaultFactor,
                                       OSMode defaultMode,
                                       bool includeRenderOptions = true,
                                       const juce::String& paramPrefix = "os");

    /** Prepares the oversamplers to process a new stream of audio */
    void prepareToPlay (double sampleRate, int samplesPerBlock);

    /** Resets the state of the oversamplers */
    void reset();

    /** Returns the oversampling factor currently in use */
    int getOSFactor() const noexcept { return (int) oversamplers[curOS]->getOversamplingFactor(); }

    /** Call this method tbefore processing a block of audio to check if a different oversampler should be used */
    bool updateOSFactor();

    /** Returns the samples of latency introduced by the oversampling process */
    float getLatencySamples() const noexcept { return (float) oversamplers[curOS]->getLatencyInSamples(); }

    /** Returns the current latency introduced by the oversampling process in milliseconds */
    float getLatencyMilliseconds() const noexcept { return ((float) oversamplers[curOS]->getLatencyInSamples() / sampleRate) * 1000.0f; }

    /** Upsample a new block of data */
    auto processSamplesUp (const juce::dsp::AudioBlock<const FloatType>& inputBlock) noexcept { return oversamplers[curOS]->processSamplesUp (inputBlock); }

    /** Downsample the last block of data into the given output block */
    void processSamplesDown (juce::dsp::AudioBlock<FloatType>& outputBlock) noexcept { oversamplers[curOS]->processSamplesDown (outputBlock); }

private:
    juce::AudioParameterChoice* osParam = nullptr;
    juce::AudioParameterChoice* osModeParam = nullptr;
    juce::AudioParameterChoice* osOfflineParam = nullptr;
    juce::AudioParameterChoice* osOfflineModeParam = nullptr;
    juce::AudioParameterBool* osOfflineSameParam = nullptr;

    int curOS = 0, prevOS = 0;
    int numOSChoices = 0;
    float sampleRate = 48000.0f;

    juce::OwnedArray<juce::dsp::Oversampling<FloatType>> oversamplers;

    const juce::AudioProcessor& proc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VariableOversampling)
};
} // namespace chowdsp
