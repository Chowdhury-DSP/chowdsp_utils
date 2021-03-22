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

namespace chowdsp
{
/** Base class for delay lines with any interpolation type */
template <typename SampleType>
class DelayLineBase
{
public:
    DelayLineBase() = default;

    virtual void setDelay (SampleType /* newDelayInSamples */) = 0;
    virtual SampleType getDelay() const = 0;

    virtual void prepare (const juce::dsp::ProcessSpec& /* spec */) = 0;
    virtual void reset() = 0;

    virtual void pushSample (int /* channel */, SampleType /* sample */) noexcept = 0;
    virtual SampleType popSample (int /* channel */) noexcept = 0;
    virtual SampleType popSample (int /* channel */, SampleType /* delayInSamples */, bool /* updateReadPointer */) noexcept = 0;

    void copyState (const DelayLineBase<SampleType>& other)
    {
        bufferData.makeCopyOf (other.bufferData);

        if (v.empty() || other.v.empty()) // nothing to copy!
            return;

        std::copy (other.v.begin(), other.v.end(), v.begin());
        std::copy (other.writePos.begin(), other.writePos.end(), writePos.begin());
        std::copy (other.readPos.begin(), other.readPos.end(), readPos.begin());
    }

protected:
    juce::AudioBuffer<SampleType> bufferData;
    std::vector<SampleType> v;
    std::vector<int> writePos, readPos;
};

//==============================================================================
/**
    A delay line processor featuring several algorithms for the fractional delay
    calculation, block processing, and sample-by-sample processing useful when
    modulating the delay in real time or creating a standard delay effect with
    feedback.

    This implementation has been modified from the original JUCE implementation
    to include 5th-order Lagrange Interpolation.

    Note: If you intend to change the delay in real time, you may want to smooth
    changes to the delay systematically using either a ramp or a low-pass filter.

    @see SmoothedValue, FirstOrderTPTFilter
*/
template <typename SampleType, typename InterpolationType = DelayLineInterpolationTypes::Linear>
class DelayLine : public DelayLineBase<SampleType>
{
public:
    //==============================================================================
    /** Default constructor. */
    DelayLine();

    /** Constructor. */
    explicit DelayLine (int maximumDelayInSamples);

    //==============================================================================
    /** Sets the delay in samples. */
    void setDelay (SampleType newDelayInSamples) override;

    /** Returns the current delay in samples. */
    SampleType getDelay() const override;

    //==============================================================================
    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec) override;

    /** Resets the internal state variables of the processor. */
    void reset() override;

    //==============================================================================
    /** Pushes a single sample into one channel of the delay line.

        Use this function and popSample instead of process if you need to modulate
        the delay in real time instead of using a fixed delay value, or if you want
        to code a delay effect with a feedback loop.

        @see setDelay, popSample, process
    */
    inline void pushSample (int channel, SampleType sample) noexcept override
    {
        const auto writePtr = this->writePos[(size_t) channel];
        bufferPtr[channel][writePtr] = sample;
        bufferPtr[channel][writePtr + totalSize] = sample;
        this->writePos[(size_t) channel] = (this->writePos[(size_t) channel] + totalSize - 1) % totalSize;
    }

    /** Pops a single sample from one channel of the delay line.

        Use this function to modulate the delay in real time or implement standard
        delay effects with feedback.

        @param channel              the target channel for the delay line.

        @param delayInSamples       sets the wanted fractional delay in samples, or -1
                                    to use the value being used before or set with
                                    setDelay function.

        @param updateReadPointer    should be set to true if you use the function
                                    once for each sample, or false if you need
                                    multi-tap delay capabilities.

        @see setDelay, pushSample, process
    */
    inline SampleType popSample (int channel) noexcept override
    {
        auto result = interpolateSample (channel);
        this->readPos[(size_t) channel] = (this->readPos[(size_t) channel] + totalSize - 1) % totalSize;

        return result;
    }

    inline SampleType popSample (int channel, SampleType delayInSamples, bool updateReadPointer) noexcept override
    {
        setDelay (delayInSamples);

        auto result = interpolateSample (channel);

        if (updateReadPointer)
            this->readPos[(size_t) channel] = (this->readPos[(size_t) channel] + totalSize - 1) % totalSize;

        return result;
    }

    //==============================================================================
    /** Processes the input and output samples supplied in the processing context.

        Can be used for block processing when the delay is not going to change
        during processing. The delay must first be set by calling setDelay.

        @see setDelay
    */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumChannels() == this->writePos.size());
        jassert (inputBlock.getNumSamples() == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples = inputBlock.getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                pushSample ((int) channel, inputSamples[i]);
                outputSamples[i] = popSample ((int) channel);
            }
        }
    }

private:
    inline SampleType interpolateSample (int channel) noexcept
    {
        auto index = (this->readPos[(size_t) channel] + delayInt);
        return interpolator.call (bufferPtr[channel],
                                  index,
                                  delayFrac,
                                  this->v[(size_t) channel]);
    }

    //==============================================================================
    InterpolationType interpolator;
    SampleType** bufferPtr = nullptr;
    SampleType delay = 0.0, delayFrac = 0.0;
    int delayInt = 0, totalSize = 4;
};

} // namespace chowdsp

#include "chowdsp_DelayLine.cpp"
