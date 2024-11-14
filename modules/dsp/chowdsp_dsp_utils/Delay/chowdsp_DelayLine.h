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
#ifndef DOXYGEN
/** Base class for delay lines with any interpolation type */
template <typename SampleType, typename StorageType = SampleType>
class DelayLineBase
{
public:
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

    DelayLineBase() = default;
    virtual ~DelayLineBase() = default;

    virtual void setDelay (NumericType /* newDelayInSamples */) = 0;
    [[nodiscard]] virtual NumericType getDelay() const = 0;

    virtual void prepare (const juce::dsp::ProcessSpec& /* spec */) = 0;
    virtual void free() = 0;
    virtual void reset() = 0;

    virtual void pushSample (int /* channel */, SampleType /* sample */) noexcept = 0;
    virtual SampleType popSample (int /* channel */) noexcept = 0;
    virtual SampleType popSample (int /* channel */, NumericType /* delayInSamples */, bool /* updateReadPointer */) noexcept = 0;
    virtual void incrementReadPointer (int channel) noexcept = 0;

    void copyState (const DelayLineBase& other)
    {
        const auto numChannels = other.bufferData.getNumChannels();
        const auto numSamples = other.bufferData.getNumSamples();
        if (numChannels != bufferData.getNumChannels()
            || numSamples != bufferData.getNumSamples())
        {
            bufferData.setMaxSize (numChannels, numSamples);
        }

        BufferMath::copyBufferData (other.bufferData, bufferData);

        if (v.empty() || other.v.empty()) // nothing to copy!
            return;

        std::copy (other.v.begin(), other.v.end(), v.begin());
        std::copy (other.writePos.begin(), other.writePos.end(), writePos.begin());
        std::copy (other.readPos.begin(), other.readPos.end(), readPos.begin());
    }

    [[nodiscard]] BufferView<SampleType> getRawDelayBuffer() { return bufferData; }
    [[nodiscard]] BufferView<const SampleType> getRawDelayBuffer() const { return bufferData; }

protected:
    Buffer<StorageType> bufferData;
    std::vector<SampleType> v;
    std::vector<int> writePos, readPos;
};
#endif // DOXYGEN

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
*/
template <typename SampleType, typename InterpolationType = DelayLineInterpolationTypes::Linear, typename StorageType = SampleType>
class DelayLine : public DelayLineBase<SampleType, StorageType>
{
    using NumericType = SampleTypeHelpers::ProcessorNumericType<DelayLine>;

public:
    //==============================================================================
    /** Default constructor. */
    DelayLine();

    /** Constructor. */
    explicit DelayLine (int maximumDelayInSamples);

    //==============================================================================
    /** Sets the delay in samples. */
    void setDelay (NumericType newDelayInSamples) final;

    /** Returns the current delay in samples. */
    NumericType getDelay() const final;

    //==============================================================================
    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec) final;

    /** Frees internal memory. */
    void free() final;

    /** Resets the internal state variables of the processor. */
    void reset() final;

    //==============================================================================
    /** Pushes a single sample into one channel of the delay line.

        Use this function and popSample instead of process if you need to modulate
        the delay in real time instead of using a fixed delay value, or if you want
        to code a delay effect with a feedback loop.

        @see setDelay, popSample, process
    */
    inline void pushSample (int channel, SampleType sample) noexcept final
    {
        const auto writePtr = this->writePos[(size_t) channel];
        bufferPtrs[(size_t) channel][writePtr] = static_cast<StorageType> (sample);
        bufferPtrs[(size_t) channel][writePtr + totalSize] = static_cast<StorageType> (sample);
        incrementWritePointer (channel);
    }

    /** Pops a single sample from one channel of the delay line.

        Use this function to modulate the delay in real time or implement standard
        delay effects with feedback.

        @param channel              the target channel for the delay line.

        @see setDelay, pushSample, process
    */
    inline SampleType popSample (int channel) noexcept final
    {
        auto result = interpolateSample (channel);
        incrementReadPointer (channel);

        return result;
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
    inline SampleType popSample (int channel, NumericType delayInSamples, bool updateReadPointer) noexcept final
    {
        setDelay (delayInSamples);

        auto result = interpolateSample (channel);

        if (updateReadPointer)
            incrementReadPointer (channel);

        return result;
    }

    /** Increment the read pointer without reading an interpolated sample (be careful...) */
    inline void incrementReadPointer (int channel) noexcept final
    {
        auto newReadPtr = this->readPos[(size_t) channel] + totalSize - 1;
        newReadPtr = newReadPtr > totalSize ? newReadPtr - totalSize : newReadPtr;
        this->readPos[(size_t) channel] = newReadPtr;
    }

    /** Process a block of audio. */
    void processBlock (const BufferView<SampleType>& buffer)
    {
        for (auto [channelIndex, channelData] : buffer_iters::channels (buffer))
        {
            for (auto& sample : channelData)
            {
                pushSample (channelIndex, sample);
                sample = popSample (channelIndex);
            }
        }
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
        return interpolator.call (bufferPtrs[(size_t) channel],
                                  index,
                                  delayFrac,
                                  this->v[(size_t) channel]);
    }

    /** Increment the write pointer (be careful...) */
    inline void incrementWritePointer (int channel) noexcept
    {
        auto newWritePtr = this->writePos[(size_t) channel] + totalSize - 1;
        newWritePtr = newWritePtr >= totalSize ? newWritePtr - totalSize : newWritePtr;
        this->writePos[(size_t) channel] = newWritePtr;
    }

    //==============================================================================
    InterpolationType interpolator;
    std::vector<StorageType*> bufferPtrs;
    NumericType delay = 0.0, delayFrac = 0.0;
    int delayInt = 0, totalSize = 4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayLine)
};

} // namespace chowdsp

#include "chowdsp_DelayLine.cpp"
