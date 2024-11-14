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
template <typename SampleType, typename DelayInterpType = NullType, typename = void>
class BypassProcessor;

template <typename SampleType, typename DelayInterpType>
class BypassProcessor<SampleType, DelayInterpType, std::enable_if_t<std::is_same_v<DelayInterpType, NullType>>>
{
public:
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

    BypassProcessor() = default;

    /** Converts a parameter handle to a boolean */
    static bool toBool (const std::atomic<float>* param)
    {
        return ! juce::approximatelyEqual (param->load(), 0.0f);
    }

    /** Allocated required memory, and resets the property */
    void prepare (const juce::dsp::ProcessSpec& spec, bool onOffParam, bool useInternalBuffer = true);

    /** Reset's the processor state. */
    void reset (bool onOffParam) { prevOnOffParam = onOffParam; }

    /**
      * Call this at the start of your processBlock().
      * If it returns false, you can safely skip all other
      * processing.
      */
    bool processBlockIn (const BufferView<const SampleType>& buffer, bool onOffParam, std::optional<ArenaAllocatorView> arena = std::nullopt);

    /**
      * Call this at the end of your processBlock().
      * It will fade the dry signal back in with the main
      * signal as needed.
      */
    void processBlockOut (const BufferView<SampleType>& buffer, bool onOffParam);

private:
    bool prevOnOffParam = false;
    Buffer<SampleType> fadeBuffer;
    BufferView<SampleType> fadeBufferView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassProcessor)
};

template <typename SampleType, typename DelayInterpType>
class BypassProcessor<SampleType, DelayInterpType, std::enable_if_t<! std::is_same_v<DelayInterpType, NullType>>>
{
public:
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

    explicit BypassProcessor (int maxLatencySamples = 1 << 18);

    /** Converts a parameter handle to a boolean */
    static bool toBool (const std::atomic<float>* param)
    {
        return ! juce::approximatelyEqual (param->load(), 0.0f);
    }

    /** Allocated required memory, and resets the property */
    void prepare (const juce::dsp::ProcessSpec& spec, bool onOffParam, bool useInternalBuffer = true);

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
    void setLatencySamples (NumericType delaySamples);

    /**
      * Call this at the start of your processBlock().
      * If it returns false, you can safely skip all other
      * processing.
      */
    bool processBlockIn (const BufferView<SampleType>& buffer, bool onOffParam, std::optional<ArenaAllocatorView> arena = std::nullopt);

    /**
      * Call this at the end of your processBlock().
      * It will fade the dry signal back in with the main
      * signal as needed.
      */
    void processBlockOut (const BufferView<SampleType>& buffer, bool onOffParam);

private:
    void setLatencySamplesInternal (NumericType delaySamples);
    int getFadeStartSample (const int numSamples);

    bool prevOnOffParam = false;
    Buffer<SampleType> fadeBuffer;
    BufferView<SampleType> fadeBufferView;

    int maximumLatencySamples = 0;
    std::optional<DelayLine<SampleType, DelayInterpType>> compDelay { std::nullopt };
    NumericType prevDelay {};
    int latencySampleCount = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassProcessor)
};

} // namespace chowdsp

#include "chowdsp_BypassProcessor.cpp"
