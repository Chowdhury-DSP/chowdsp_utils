/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

namespace chowdsp
{
/** Panning rules: determine the balance of left/right channels at a given pan setting */
enum class PanningRule
{
    linear, /**< regular 6 dB or linear panning rule, allows the panned sound to be
                          perceived as having a constant level when summed to mono */
    balanced, /**< both left and right are 1 when pan value is 0.5, with left decreasing
                          to 0 above this value and right decreasing to 0 below it */
    sin3dB, /**< alternate version of the regular 3 dB panning rule with a sine curve */
    sin4p5dB, /**< alternate version of the regular 4.5 dB panning rule with a sine curve */
    sin6dB, /**< alternate version of the regular 6 dB panning rule with a sine curve */
    squareRoot3dB, /**< regular 3 dB or constant power panning rule, allows the panned sound
                          to be perceived as having a constant level regardless of the pan position */
    squareRoot4p5dB /**< regular 4.5 dB panning rule, a compromise option between 3 dB and 6 dB panning rules */
};

/**
    A processor to perform panning operations on stereo buffers.

    Modified from the juce::dsp::Panner to include single-sample processing.
*/
template <typename SampleType>
class Panner
{
public:
    /** Panning rules: determine the balance of left/right channels at a given pan setting */
    using Rule = PanningRule;

    //==============================================================================
    /** Constructor. */
    Panner();

    //==============================================================================
    /** Sets the panning rule. */
    void setRule (Rule newRule);

    /** Sets the current panning value, between -1 (full left) and 1 (full right). */
    void setPan (SampleType newPan);

    //==============================================================================
    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset();

    void processBlock (const BufferView<SampleType>& buffer) noexcept
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        jassert (numChannels == 2);
        juce::ignoreUnused (numChannels);

        BufferView<SampleType> leftChannelBuffer { buffer, 0, numSamples, 0, 1 };
        BufferView<SampleType> rightChannelBuffer { buffer, 0, numSamples, 1, 1 };

        BufferMath::applyGainSmoothed (leftChannelBuffer, leftVolume);
        BufferMath::applyGainSmoothed (rightChannelBuffer, rightVolume);
    }

    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        const auto numInputChannels = inputBlock.getNumChannels();
        const auto numOutputChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        jassert (inputBlock.getNumSamples() == numSamples);
        juce::ignoreUnused (numSamples);

        if (numOutputChannels != 2 || numInputChannels == 0 || numInputChannels > 2)
            return;

        if (numInputChannels == 2)
        {
            outputBlock.copyFrom (inputBlock);
        }
        else
        {
            outputBlock.getSingleChannelBlock (0).copyFrom (inputBlock);
            outputBlock.getSingleChannelBlock (1).copyFrom (inputBlock);
        }

        if (context.isBypassed)
            return;

        outputBlock.getSingleChannelBlock (0).multiplyBy (leftVolume);
        outputBlock.getSingleChannelBlock (1).multiplyBy (rightVolume);
    }

    /** 
     * A function for panning single samples. This is useful if you have
     * some sort of "modulating" pan effect.
     * 
     * An example of how this can be used:
     * ```
     * chowdsp::Panner<float> panner;
     * juce::AudioBuffer<float> monoBuffer;
     * juce::AudioBuffer<float> stereoBuffer;
     * 
     * auto* input = monoBuffer.getReadPointer (0);
     * auto* left  = stereoBuffer.getWritePointer (0);
     * auto* right = stereoBuffer.getWritePointer (1);
     * 
     * for (int i = 0; i < monoBuffer.getNumSamples(); ++i)
     * {
     *     panner.setPan (newPanValue);
     *     std::tie (left[i], right[i]) = panner.processSample (input[i]);
     * }
     * ```
    */
    inline std::pair<SampleType, SampleType> processSample (SampleType x) noexcept
    {
        return { x * leftVolume.getNextValue(), x * rightVolume.getNextValue() };
    }

    static SampleType getBoostForRule (Rule rule);

private:
    //==============================================================================
    void update();

    //==============================================================================
    Rule currentRule = Rule::balanced;
    SampleType pan = 0.0;
    juce::SmoothedValue<SampleType> leftVolume, rightVolume;
    double sampleRate = 44100.0;
};

} // namespace chowdsp
