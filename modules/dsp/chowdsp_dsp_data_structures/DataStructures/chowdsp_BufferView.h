#pragma once

namespace chowdsp
{
/**
 * A "view" into an audio buffer that has already had its memory
 * allocated. Can be constructed from a chowdsp::Buffer,
 * juce::AudioBuffer, juce::dsp::AudioBlock, or raw memory.
 */
template <typename SampleType>
class BufferView
{
public:
    /** The sample type used by the buffer */
    using Type = SampleType;

    BufferView (SampleType* const* data, int dataNumChannels, int dataNumSamples, int sampleOffset = 0) : numChannels (dataNumChannels),
                                                                                                          numSamples (dataNumSamples)
    {
        initialise (data, sampleOffset);
    }

    BufferView (Buffer<SampleType>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        jassert (buffer.getNumChannels() >= startChannel + numChannels);
        jassert (buffer.getNumSamples() >= sampleOffset + numSamples);
        initialise (buffer.getArrayOfWritePointers(), sampleOffset, startChannel);
    }

    BufferView (const BufferView<SampleType>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        jassert (buffer.getNumChannels() >= startChannel + numChannels);
        jassert (buffer.getNumSamples() >= sampleOffset + numSamples);
        initialise (buffer.getArrayOfWritePointers(), sampleOffset, startChannel);
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

    /** Clears memory within the buffer view. */
    void clear() noexcept
    {
        buffer_detail::clear (channelPointers.data(), 0, numChannels, 0, numSamples);
    }

    /** Returns the number of channels in the buffer view. */
    [[nodiscard]] int getNumChannels() const noexcept { return numChannels; }

    /** Returns the number of samples in the buffer view. */
    [[nodiscard]] int getNumSamples() const noexcept { return numSamples; }

    /** Returns a pointer which can be used to write to a single channel of the buffer view. */
    SampleType* getWritePointer (int channel) const noexcept { return channelPointers[(size_t) channel]; }

    /** Returns a pointer which can be used to read from a single channel of the buffer view. */
    const SampleType* getReadPointer (int channel) const noexcept { return channelPointers[(size_t) channel]; }

    /** Returns the entire buffer view as an array of pointers to each channel's data. */
    SampleType* const* getArrayOfWritePointers() const noexcept { return channelPointers.data(); }

    /** Returns the entire buffer view as an array of pointers to each channel's data. */
    const SampleType** getArrayOfReadPointers() const noexcept { return const_cast<const SampleType**> (channelPointers.data()); }

private:
    void initialise (SampleType* const* data, int sampleOffset, int startChannel = 0)
    {
        for (size_t ch = 0; ch < (size_t) numChannels; ++ch)
            channelPointers[ch] = data[ch + (size_t) startChannel] + sampleOffset;
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
