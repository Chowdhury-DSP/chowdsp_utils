namespace chowdsp
{
/** Utility class for *smoothly* bypassing a processor */
template <typename SampleType>
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
    void prepare (int samplesPerBlock, bool onOffParam)
    {
        prevOnOffParam = onOffParam;
        fadeBuffer.setSize (2, samplesPerBlock);
        fadeBlock = juce::dsp::AudioBlock<SampleType> (fadeBuffer);
    }

    /**
      * Call this at the start of your processBlock().
      * If it returns false, you can safely skip all other
      * processing.
      */
    bool processBlockIn (const juce::AudioBuffer<SampleType>& block, bool onOffParam)
    {
        if (onOffParam == false && prevOnOffParam == false)
            return false;

        if (onOffParam != prevOnOffParam)
            fadeBuffer.makeCopyOf (block, true);

        return true;
    }

    /**
      * Call this at the start of your processBlock().
      * If it returns false, you can safely skip all other
      * processing.
      */
    bool processBlockIn (const juce::dsp::AudioBlock<SampleType>& block, bool onOffParam)
    {
        if (onOffParam == false && prevOnOffParam == false)
            return false;

        if (onOffParam != prevOnOffParam)
            fadeBlock.copyFrom (block);

        return true;
    }

    /**
      * Call this at the end of your processBlock().
      * It will fade the dry signal back in with the main
      * signal as needed.
      */
    void processBlockOut (juce::AudioBuffer<SampleType>& block, bool onOffParam)
    {
        if (onOffParam == prevOnOffParam)
            return;

        const auto numChannels = block.getNumChannels();
        const auto numSamples = block.getNumSamples();

        SampleType startGain = onOffParam == false ? static_cast<SampleType> (1) // fade out
                                                   : static_cast<SampleType> (0); // fade in
        SampleType endGain = static_cast<SampleType> (1) - startGain;

        block.applyGainRamp (0, numSamples, startGain, endGain);
        for (int ch = 0; ch < numChannels; ++ch)
            block.addFromWithRamp (ch, 0, fadeBuffer.getReadPointer (ch), numSamples, 1.0f - startGain, 1.0f - endGain);

        prevOnOffParam = onOffParam;
    }

    /**
      * Call this at the end of your processBlock().
      * It will fade the dry signal back in with the main
      * signal as needed.
      */
    void processBlockOut (juce::dsp::AudioBlock<float>& block, bool onOffParam)
    {
        if (onOffParam == prevOnOffParam)
            return;

        const auto numChannels = block.getNumChannels();
        const auto numSamples = block.getNumSamples();

        SampleType startGain = onOffParam == false ? static_cast<SampleType> (1) // fade out
                                                   : static_cast<SampleType> (0); // fade in
        SampleType endGain = static_cast<SampleType> (1) - startGain;

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* blockPtr = block.getChannelPointer (ch);
            auto* fadePtr = fadeBlock.getChannelPointer (ch);

            SampleType gain = startGain;
            SampleType increment = (endGain - startGain) / (SampleType) numSamples;

            for (size_t n = 0; n < numSamples; ++n)
            {
                blockPtr[n] = blockPtr[n] * gain + fadePtr[n] * (static_cast<SampleType> (1) - gain);
                gain += increment;
            }
        }

        prevOnOffParam = onOffParam;
    }

private:
    bool prevOnOffParam = false;
    juce::AudioBuffer<SampleType> fadeBuffer;
    juce::dsp::AudioBlock<SampleType> fadeBlock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassProcessor)
};

} // namespace chowdsp
