#pragma once

namespace chowdsp
{
template <typename SampleType>
class BufferView
{
public:
    /** The sample type used by the buffer */
    using Type = SampleType;
    
    BufferView (SampleType** data, int dataNumChannels, int dataNumSamples, int sampleOffset = 0) : numChannels (dataNumChannels),
                                                                                                    numSamples (dataNumSamples)
    {
        initialise (data, sampleOffset);
    }

    BufferView (Buffer<SampleType>& buffer, int sampleOffset = 0) // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
        : numChannels (buffer.getNumChannels()),
          numSamples (buffer.getNumSamples() - sampleOffset)
    {
        initialise (buffer.getArrayOfWritePointers(), sampleOffset);
    }

#if CHOWDSP_USING_JUCE
    BufferView (juce::AudioBuffer<SampleType>& buffer, int sampleOffset = 0) // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
        : numChannels (buffer.getNumChannels()),
          numSamples (buffer.getNumSamples() - sampleOffset)
    {
        initialise (buffer.getArrayOfWritePointers(), sampleOffset);
    }

#if JUCE_MODULE_AVAILABLE_juce_dsp
    BufferView (chowdsp::AudioBlock<SampleType>& block, int sampleOffset = 0) // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
        : numChannels ((int) block.getNumChannels()),
          numSamples ((int) block.getNumSamples() - sampleOffset)
    {
        for (size_t ch = 0; ch < (size_t) numChannels; ++ch)
            channelPointers[ch] = block.getChannelPointer (ch) + sampleOffset;
    }
#endif
#endif

    void clear() noexcept
    {
        buffer_detail::clear (channelPointers.data(), 0, numChannels, 0, numSamples);
    }

    [[nodiscard]] int getNumChannels() const noexcept { return numChannels; }
    [[nodiscard]] int getNumSamples() const noexcept { return numSamples; }

    SampleType* getWritePointer (int channel) const noexcept { return channelPointers[(size_t) channel]; }
    const SampleType* getReadPointer (int channel) const noexcept { return channelPointers[(size_t) channel]; }

    SampleType* const* getArrayOfWritePointers() const noexcept { return channelPointers.data(); }
    const SampleType** getArrayOfReadPointers() const noexcept { return const_cast<const SampleType**> (channelPointers.data()); }

private:
    void initialise (SampleType** data, int sampleOffset)
    {
        for (size_t ch = 0; ch < (size_t) numChannels; ++ch)
            channelPointers[ch] = data[ch] + sampleOffset;
    }

    const int numChannels;
    const int numSamples;

    // Assuming we will never need an audio buffer with more than 64 channels.
    // Maybe we'll need to increase this is we're doing high-order ambisonics or something?
    static constexpr int maxNumChannels = 64;
    std::array<SampleType*, (size_t) maxNumChannels> channelPointers {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferView)
};
} // namespace chowdsp
