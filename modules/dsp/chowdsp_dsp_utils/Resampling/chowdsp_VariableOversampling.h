#pragma once

#include <chowdsp_listeners/chowdsp_listeners.h>

namespace chowdsp
{
/**
 * Let's say you've got a plugin that should have oversampling options.
 * This class can be used to create a set of oversamplers connected to
 * parameters that manage the oversampling factor and mode, and may also
 * be used to manage a separate oversampling mode for offline rendering.
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
     * @param useIntegerLatency Set to true to force the oversamplers to use integer latency
     * @param paramPrefix       The same parameter prefix used to create the parameters
     */
    explicit VariableOversampling (const juce::AudioProcessorValueTreeState& vts, bool useIntegerLatency = false, const juce::String& paramPrefix = "os");

    /**
     * Creates a parameter layout for variable oversampling,
     * with all available oversampling options.
     *
     * @param params                Vector of parameters to add to
     * @param defaultFactor         Default oversampling factor
     * @param defaultMode           Default oversampling mode
     * @param versionHint           For JUCE7+, use this argument to set a version hint for the parameters
     * @param includeRenderOptions  Set to true to include parameters for oversampling when doing offline rendering
     * @param paramPrefix           Parameter prefix to use for parameter IDs
     */
    static void createParameterLayout (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params,
                                       OSFactor defaultFactor,
                                       OSMode defaultMode,
                                       int versionHint = 0,
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
                                       int versionHint = 0,
                                       bool includeRenderOptions = true,
                                       const juce::String& paramPrefix = "os");

    /** Prepares the oversamplers to process a new stream of audio */
    void prepareToPlay (double sampleRate, int samplesPerBlock, int numChannels);

    /** Resets the state of the oversamplers */
    void reset();

    /** Returns true if the oversampling processors have been prepared */
    [[nodiscard]] bool hasBeenPrepared() const noexcept { return ! oversamplers.isEmpty(); }

    /** Returns the oversampling factor currently in use */
    [[nodiscard]] int getOSFactor() const noexcept { return (int) oversamplers[curOS]->getOversamplingFactor(); }

    /** Call this method tbefore processing a block of audio to check if a different oversampler should be used */
    bool updateOSFactor();

    /** Get the index of a given oversampler */
    [[nodiscard]] int getOSIndex (int osFactor, int osMode) const noexcept { return osFactor + (numOSChoices * osMode); }

    /** Returns the samples of latency introduced by the oversampling process */
    [[nodiscard]] float getLatencySamples() const noexcept;

    /**
     * Returns the current latency introduced by the oversampling process in milliseconds,
     * or provide the oversampler index to get the latency of one specific oversampler.
     */
    [[nodiscard]] float getLatencyMilliseconds (int osIndex = -1) const noexcept;

    /** Upsample a new block of data */
    auto processSamplesUp (const AudioBlock<const FloatType>& inputBlock) noexcept { return oversamplers[curOS]->processSamplesUp (inputBlock); }

    /** Downsample the last block of data into the given output block */
    void processSamplesDown (AudioBlock<FloatType>& outputBlock) noexcept { oversamplers[curOS]->processSamplesDown (outputBlock); }

    /** Returns the set of parameters used by the oversamplers */
    auto getParameters() { return std::tie (osParam, osModeParam, osOfflineParam, osOfflineModeParam, osOfflineSameParam); }

    /** Broadcaster that is triggered when the sample rate or block size changes */
    Broadcaster<void()> sampleRateOrBlockSizeChangedBroadcaster;

private:
    static OSFactor stringToOSFactor (const juce::String& factorStr);
    static OSMode stringToOSMode (const juce::String& modeStr);
    static juce::String osFactorToString (OSFactor factor);
    static juce::String osModeToString (OSMode mode);

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

    const bool usingIntegerLatency;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VariableOversampling)
};
} // namespace chowdsp
