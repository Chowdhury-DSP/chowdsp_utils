#pragma once

namespace chowdsp
{
/**
 * Utility class for *smoothly* bypassing a processor,
 * including latency compensation.
 * 
 * BypassProcessor should be used as follows:
 * ```
 * BypassProcessor bypass;
 * std::atomic<float>* onOffParam;
 * 
 * void processBlock (AudioBuffer<float> buffer)
 * {
 *     if (! bypass.processBlockIn (buffer, bypass.toBool (onOffParam)))
 *         return;
 * 
 *     // do my processing here....
 *   
 *     bypass.processBlockOut (buffer, bypass.toBool (onOffParam));
 * }
 * ```
 */
template <typename SampleType, typename DelayInterpType = DelayLineInterpolationTypes::None>
class BypassProcessor
{
public:
    BypassProcessor() = default;

    /** Converts a parameter handle to a boolean */
    static bool toBool (const std::atomic<float>* param)
    {
        return static_cast<bool> (param->load());
    }

    /** Allocated required memory, and resets the property */
    void prepare (const juce::dsp::ProcessSpec& spec, bool onOffParam);

    /**
     * If the non-bypassed processing has some associated
     * latency, it is recommended to report the latency
     * time here, so that the bypass processing will be
     * correctly time-aligned.
     */
    void setLatencySamples (int delaySamples);

    /**
     * If the non-bypassed processing has some associated
     * latency, it is recommended to report the latency
     * time here, so that the bypass processing will be
     * correctly time-aligned.
     */
    void setLatencySamples (SampleType delaySamples);

    /**
      * Call this at the start of your processBlock().
      * If it returns false, you can safely skip all other
      * processing.
      */
    bool processBlockIn (juce::AudioBuffer<SampleType>& block, bool onOffParam);

    /**
      * Call this at the start of your processBlock().
      * If it returns false, you can safely skip all other
      * processing.
      */
    bool processBlockIn (const chowdsp::AudioBlock<SampleType>& block, bool onOffParam);

    /**
      * Call this at the end of your processBlock().
      * It will fade the dry signal back in with the main
      * signal as needed.
      */
    void processBlockOut (juce::AudioBuffer<SampleType>& block, bool onOffParam);

    /**
      * Call this at the end of your processBlock().
      * It will fade the dry signal back in with the main
      * signal as needed.
      */
    void processBlockOut (chowdsp::AudioBlock<SampleType>& block, bool onOffParam);

private:
    void setLatencySamplesInternal (SampleType delaySamples);
    int getFadeStartSample (const int numSamples);

    bool prevOnOffParam = false;
    juce::AudioBuffer<SampleType> fadeBuffer;
    chowdsp::AudioBlock<SampleType> fadeBlock;

    DelayLine<SampleType, DelayInterpType> compDelay { 1 << 18 }; // max latency = 2^18 = 262144 samples
    SampleType prevDelay {};
    int latencySampleCount = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassProcessor)
};

} // namespace chowdsp

#include "chowdsp_BypassProcessor.cpp"
