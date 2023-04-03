#pragma once

namespace chowdsp
{
/**
    Minimal and lightweight data-structure which contains a list of pointers to
    channels containing some kind of sample data.

    This class doesn't own any of the data which it points to, it's simply a view
    into data that is owned elsewhere. You can construct one from some raw data
    that you've allocated yourself, or give it a HeapBlock to use, or give it
    an AudioBuffer which it can refer to, but in all cases the user is
    responsible for making sure that the data doesn't get deleted while there's
    still an AudioBlock using it.
*/
template <typename SampleType>
class SIMDAudioBlock
{
private:
    template <typename OtherSampleType>
    using MayUseConvertingConstructor =
        std::enable_if_t<std::is_same<std::remove_const_t<SampleType>,
                                      std::remove_const_t<OtherSampleType>>::value
                             && std::is_const<SampleType>::value
                             && ! std::is_const<OtherSampleType>::value,
                         int>;

public:
    //==============================================================================
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

    //==============================================================================
    /** Create a zero-sized AudioBlock. */
    SIMDAudioBlock() noexcept = default;

    /** Creates an AudioBlock from a pointer to an array of channels.
        AudioBlock does not copy nor own the memory pointed to by dataToUse.
        Therefore it is the user's responsibility to ensure that the memory is retained
        throughout the life-time of the AudioBlock and released when no longer needed.
    */
    constexpr SIMDAudioBlock (SampleType* const* channelData,
                              size_t numberOfChannels,
                              size_t numberOfSamples) noexcept
        : channels (channelData),
          numChannels (static_cast<ChannelCountType> (numberOfChannels)),
          numSamples (numberOfSamples)
    {
    }

    /** Creates an AudioBlock from a pointer to an array of channels.
        AudioBlock does not copy nor own the memory pointed to by dataToUse.
        Therefore it is the user's responsibility to ensure that the memory is retained
        throughout the life-time of the AudioBlock and released when no longer needed.
    */
    constexpr SIMDAudioBlock (SampleType* const* channelData, size_t numberOfChannels, size_t startSampleIndex, size_t numberOfSamples) noexcept
        : channels (channelData),
          numChannels (static_cast<ChannelCountType> (numberOfChannels)),
          startSample (startSampleIndex),
          numSamples (numberOfSamples)
    {
    }

    /** Allocates a suitable amount of space in a HeapBlock, and initialises this object
        to point into it.
        The HeapBlock must of course not be freed or re-allocated while this object is still in
        use, because it will be referencing its data.
    */
    SIMDAudioBlock (juce::HeapBlock<char>& heapBlockToUseForAllocation,
                    size_t numberOfChannels,
                    size_t numberOfSamples,
                    size_t alignmentInBytes = defaultAlignment) noexcept
        : numChannels (static_cast<ChannelCountType> (numberOfChannels)),
          numSamples (numberOfSamples)
    {
        auto roundedUpNumSamples = (numberOfSamples + elementMask) & ~elementMask;
        auto channelSize = sizeof (SampleType) * roundedUpNumSamples;
        auto channelListBytes = sizeof (SampleType*) * numberOfChannels;
        auto extraBytes = alignmentInBytes - 1;

        heapBlockToUseForAllocation.malloc (channelListBytes + extraBytes + channelSize * numberOfChannels);

        auto* chanArray = juce::unalignedPointerCast<SampleType**> (heapBlockToUseForAllocation.getData());
        channels = chanArray;

        auto* data = juce::unalignedPointerCast<SampleType*> (juce::addBytesToPointer (chanArray, channelListBytes));
        data = juce::snapPointerToAlignment (data, alignmentInBytes);

        for (ChannelCountType i = 0; i < numChannels; ++i)
        {
            chanArray[i] = data;
            data += roundedUpNumSamples;
        }
    }

    SIMDAudioBlock (const SIMDAudioBlock& other) noexcept = default;
    SIMDAudioBlock& operator= (const SIMDAudioBlock& other) noexcept = default;

    template <typename OtherSampleType, MayUseConvertingConstructor<OtherSampleType> = 0>
    SIMDAudioBlock (const SIMDAudioBlock<OtherSampleType>& other) noexcept //NOLINT(google-explicit-constructor) we want to be able to use this constructor implicitly
        : channels { other.channels },
          numChannels { other.numChannels },
          startSample { other.startSample },
          numSamples { other.numSamples }
    {
    }

    template <typename OtherSampleType, MayUseConvertingConstructor<OtherSampleType> = 0>
    SIMDAudioBlock& operator= (const SIMDAudioBlock<OtherSampleType>& other) noexcept
    {
        SIMDAudioBlock blockCopy { other };
        swap (blockCopy);
        return *this;
    }

    void swap (SIMDAudioBlock& other) noexcept
    {
        std::swap (other.channels, channels);
        std::swap (other.numChannels, numChannels);
        std::swap (other.startSample, startSample);
        std::swap (other.numSamples, numSamples);
    }

    //==============================================================================
    template <typename OtherSampleType>
    constexpr bool operator== (const SIMDAudioBlock<OtherSampleType>& other) const noexcept
    {
        return std::equal (channels,
                           channels + numChannels,
                           other.channels,
                           other.channels + other.numChannels)
               && startSample == other.startSample
               && numSamples == other.numSamples;
    }

    template <typename OtherSampleType>
    constexpr bool operator!= (const SIMDAudioBlock<OtherSampleType>& other) const noexcept
    {
        return ! (*this == other);
    }

    //==============================================================================
    /** Returns the number of channels referenced by this block. */
    [[nodiscard]] constexpr size_t getNumChannels() const noexcept { return static_cast<size_t> (numChannels); }

    /** Returns the number of samples referenced by this block. */
    [[nodiscard]] constexpr size_t getNumSamples() const noexcept { return numSamples; }

    /** Returns a raw pointer into one of the channels in this block. */
    SampleType* getChannelPointer (size_t channel) const noexcept
    {
        jassert (channel < numChannels);
        jassert (numSamples > 0);
        return channels[channel] + startSample;
    }

    /** Returns an AudioBlock that represents one of the channels in this block. */
    SIMDAudioBlock getSingleChannelBlock (size_t channel) const noexcept
    {
        jassert (channel < numChannels);
        return AudioBlock (channels + channel, 1, startSample, numSamples);
    }

    /** Returns a subset of contiguous channels
        @param channelStart       First channel of the subset
        @param numChannelsToUse   Count of channels in the subset
    */
    SIMDAudioBlock getSubsetChannelBlock (size_t channelStart, size_t numChannelsToUse) const noexcept
    {
        jassert (channelStart < numChannels);
        jassert ((channelStart + numChannelsToUse) <= numChannels);

        return AudioBlock (channels + channelStart, numChannelsToUse, startSample, numSamples);
    }

    /** Returns a sample from the buffer.
        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    SampleType getSample (int channel, int sampleIndex) const noexcept
    {
        jassert (juce::isPositiveAndBelow (channel, numChannels));
        jassert (juce::isPositiveAndBelow (sampleIndex, numSamples));
        return channels[channel][(size_t) startSample + (size_t) sampleIndex];
    }

    /** Modifies a sample in the buffer.
        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    void setSample (int destChannel, int destSample, SampleType newValue) const noexcept
    {
        jassert (juce::isPositiveAndBelow (destChannel, numChannels));
        jassert (juce::isPositiveAndBelow (destSample, numSamples));
        channels[destChannel][(size_t) startSample + (size_t) destSample] = newValue;
    }

    /** Adds a value to a sample in the buffer.
        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    void addSample (int destChannel, int destSample, SampleType valueToAdd) const noexcept
    {
        jassert (juce::isPositiveAndBelow (destChannel, numChannels));
        jassert (juce::isPositiveAndBelow (destSample, numSamples));
        channels[destChannel][(size_t) startSample + (size_t) destSample] += valueToAdd;
    }

    //==============================================================================
    /** Clears the memory referenced by this AudioBlock. */
    SIMDAudioBlock& clear() noexcept
    {
        clearInternal();
        return *this;
    }
    const SIMDAudioBlock& clear() const noexcept
    {
        clearInternal();
        return *this;
    }

    /** Fills the memory referenced by this AudioBlock with value. */
    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE fill (NumericType value) noexcept
    {
        fillInternal (value);
        return *this;
    }
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE fill (NumericType value) const noexcept
    {
        fillInternal (value);
        return *this;
    }

    /** Copies the values in src to this block. */
    template <typename OtherSampleType>
    SIMDAudioBlock& copyFrom (const SIMDAudioBlock<OtherSampleType>& src) noexcept
    {
        copyFromInternal (src);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& copyFrom (const SIMDAudioBlock<OtherSampleType>& src) const noexcept
    {
        copyFromInternal (src);
        return *this;
    }

    /** Move memory within this block from the position srcPos to the position dstPos.
            If numElements is not specified then move will move the maximum amount of memory.
        */
    SIMDAudioBlock& move (size_t srcPos, size_t dstPos, size_t numElements = std::numeric_limits<size_t>::max()) noexcept
    {
        moveInternal (srcPos, dstPos, numElements);
        return *this;
    }
    const SIMDAudioBlock& move (size_t srcPos, size_t dstPos, size_t numElements = std::numeric_limits<size_t>::max()) const noexcept
    {
        moveInternal (srcPos, dstPos, numElements);
        return *this;
    }

    //==============================================================================
    /** Return a new AudioBlock pointing to a sub-block inside this block. This
            function does not copy the memory and you must ensure that the original memory
            pointed to by the receiver remains valid through-out the life-time of the
            returned sub-block.

            @param newOffset   The index of an element inside the receiver which will
                               will become the first element of the return value.
            @param newLength   The number of elements of the newly created sub-block.
        */
    SIMDAudioBlock getSubBlock (size_t newOffset, size_t newLength) const noexcept
    {
        jassert (newOffset < numSamples);
        jassert (newOffset + newLength <= numSamples);

        return SIMDAudioBlock (channels, numChannels, startSample + newOffset, newLength);
    }

    /** Return a new AudioBlock pointing to a sub-block inside this block. This
        function does not copy the memory and you must ensure that the original memory
        pointed to by the receiver remains valid through-out the life-time of the
        returned sub-block.

        @param newOffset   The index of an element inside the block which will
                           will become the first element of the return value.
                           The return value will include all subsequent elements
                           of the receiver.
    */
    SIMDAudioBlock getSubBlock (size_t newOffset) const noexcept
    {
        return getSubBlock (newOffset, getNumSamples() - newOffset);
    }

    //==============================================================================
    /** Adds a fixed value to the elements in this block. */
    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE add (NumericType value) noexcept
    {
        addInternal (value);
        return *this;
    }
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE add (NumericType value) const noexcept
    {
        addInternal (value);
        return *this;
    }

    /** Adds the elements in the src block to the elements in this block. */
    template <typename OtherSampleType>
    SIMDAudioBlock& add (SIMDAudioBlock<OtherSampleType> src) noexcept
    {
        addInternal (src);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& add (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        addInternal (src);
        return *this;
    }

    /** Adds a fixed value to each source value and replaces the contents of this block. */
    template <typename OtherSampleType>
    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE replaceWithSumOf (SIMDAudioBlock<OtherSampleType> src, NumericType value) noexcept
    {
        replaceWithSumOfInternal (src, value);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE replaceWithSumOf (SIMDAudioBlock<OtherSampleType> src, NumericType value) const noexcept
    {
        replaceWithSumOfInternal (src, value);
        return *this;
    }

    /** Adds each source1 value to the corresponding source2 value and replaces the contents of this block. */
    template <typename Src1SampleType, typename Src2SampleType>
    SIMDAudioBlock& replaceWithSumOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) noexcept
    {
        replaceWithSumOfInternal (src1, src2);
        return *this;
    }
    template <typename Src1SampleType, typename Src2SampleType>
    const SIMDAudioBlock& replaceWithSumOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        replaceWithSumOfInternal (src1, src2);
        return *this;
    }

    //==============================================================================
    /** Subtracts a fixed value from the elements in this block. */
    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE subtract (NumericType value) noexcept
    {
        subtractInternal (value);
        return *this;
    }
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE subtract (NumericType value) const noexcept
    {
        subtractInternal (value);
        return *this;
    }

    /** Subtracts the source values from the elements in this block. */
    template <typename OtherSampleType>
    SIMDAudioBlock& subtract (SIMDAudioBlock<OtherSampleType> src) noexcept
    {
        subtractInternal (src);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& subtract (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        subtractInternal (src);
        return *this;
    }

    /** Subtracts a fixed value from each source value and replaces the contents of this block. */
    template <typename OtherSampleType>
    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE replaceWithDifferenceOf (SIMDAudioBlock<OtherSampleType> src, NumericType value) noexcept
    {
        replaceWithDifferenceOfInternal (src, value);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE replaceWithDifferenceOf (SIMDAudioBlock<OtherSampleType> src, NumericType value) const noexcept
    {
        replaceWithDifferenceOfInternal (src, value);
        return *this;
    }

    /** Subtracts each source2 value from the corresponding source1 value and replaces the contents of this block. */
    template <typename Src1SampleType, typename Src2SampleType>
    SIMDAudioBlock& replaceWithDifferenceOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) noexcept
    {
        replaceWithDifferenceOfInternal (src1, src2);
        return *this;
    }
    template <typename Src1SampleType, typename Src2SampleType>
    const SIMDAudioBlock& replaceWithDifferenceOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        replaceWithDifferenceOfInternal (src1, src2);
        return *this;
    }

    //==============================================================================
    /** Multiplies the elements in this block by a fixed value. */
    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE multiplyBy (NumericType value) noexcept
    {
        multiplyByInternal (value);
        return *this;
    }
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE multiplyBy (NumericType value) const noexcept
    {
        multiplyByInternal (value);
        return *this;
    }

    /** Multiplies the elements in this block by the elements in the src block */
    template <typename OtherSampleType>
    SIMDAudioBlock& multiplyBy (SIMDAudioBlock<OtherSampleType> src) noexcept
    {
        multiplyByInternal (src);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& multiplyBy (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        multiplyByInternal (src);
        return *this;
    }

    /** Replaces the elements in this block with the product of the elements in the source src block and a fixed value. */
    template <typename OtherSampleType>
    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE replaceWithProductOf (SIMDAudioBlock<OtherSampleType> src, NumericType value) noexcept
    {
        replaceWithProductOfInternal (src, value);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE replaceWithProductOf (SIMDAudioBlock<OtherSampleType> src, NumericType value) const noexcept
    {
        replaceWithProductOfInternal (src, value);
        return *this;
    }

    /** Replaces the elements in this block with the product of the elements in the src1 and scr2 blocks. */
    template <typename Src1SampleType, typename Src2SampleType>
    SIMDAudioBlock& replaceWithProductOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) noexcept
    {
        replaceWithProductOfInternal (src1, src2);
        return *this;
    }
    template <typename Src1SampleType, typename Src2SampleType>
    const SIMDAudioBlock& replaceWithProductOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        replaceWithProductOfInternal (src1, src2);
        return *this;
    }

    //==============================================================================
    /** Multiplies each channels of this block by a smoothly changing value. */
    template <typename OtherSampleType, typename SmoothingType>
    SIMDAudioBlock& multiplyBy (juce::SmoothedValue<OtherSampleType, SmoothingType>& value) noexcept
    {
        multiplyByInternal (value);
        return *this;
    }
    template <typename OtherSampleType, typename SmoothingType>
    const SIMDAudioBlock& multiplyBy (juce::SmoothedValue<OtherSampleType, SmoothingType>& value) const noexcept
    {
        multiplyByInternal (value);
        return *this;
    }

    /** Replaces each channel of this block with the product of the src block and a smoothed value. */
    template <typename BlockSampleType, typename SmootherSampleType, typename SmoothingType>
    SIMDAudioBlock& replaceWithProductOf (SIMDAudioBlock<BlockSampleType> src, juce::SmoothedValue<SmootherSampleType, SmoothingType>& value) noexcept
    {
        replaceWithProductOfInternal (src, value);
        return *this;
    }
    template <typename BlockSampleType, typename SmootherSampleType, typename SmoothingType>
    const SIMDAudioBlock& replaceWithProductOf (SIMDAudioBlock<BlockSampleType> src, juce::SmoothedValue<SmootherSampleType, SmoothingType>& value) const noexcept
    {
        replaceWithProductOfInternal (src, value);
        return *this;
    }

    //==============================================================================
    /** Multiplies each value in src by a fixed value and adds the result to this block. */
    template <typename OtherSampleType>
    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE addProductOf (SIMDAudioBlock<OtherSampleType> src, NumericType factor) noexcept
    {
        addProductOfInternal (src, factor);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE addProductOf (SIMDAudioBlock<OtherSampleType> src, NumericType factor) const noexcept
    {
        addProductOfInternal (src, factor);
        return *this;
    }

    /** Multiplies each value in srcA with the corresponding value in srcB and adds the result to this block. */
    template <typename Src1SampleType, typename Src2SampleType>
    SIMDAudioBlock& addProductOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) noexcept
    {
        addProductOfInternal (src1, src2);
        return *this;
    }
    template <typename Src1SampleType, typename Src2SampleType>
    const SIMDAudioBlock& addProductOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        addProductOfInternal (src1, src2);
        return *this;
    }

    //==============================================================================
    /** Negates each value of this block. */
    SIMDAudioBlock& negate() noexcept
    {
        negateInternal();
        return *this;
    }
    const SIMDAudioBlock& negate() const noexcept
    {
        negateInternal();
        return *this;
    }

    /** Replaces the contents of this block with the negative of the values in the src block. */
    template <typename OtherSampleType>
    SIMDAudioBlock& replaceWithNegativeOf (SIMDAudioBlock<OtherSampleType> src) noexcept
    {
        replaceWithNegativeOfInternal (src);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& replaceWithNegativeOf (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        replaceWithNegativeOfInternal (src);
        return *this;
    }

    /** Replaces the contents of this block with the absolute values of the src block. */
    template <typename OtherSampleType>
    SIMDAudioBlock& replaceWithAbsoluteValueOf (SIMDAudioBlock<OtherSampleType> src) noexcept
    {
        replaceWithAbsoluteValueOfInternal (src);
        return *this;
    }
    template <typename OtherSampleType>
    const SIMDAudioBlock& replaceWithAbsoluteValueOf (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        replaceWithAbsoluteValueOfInternal (src);
        return *this;
    }

    //==============================================================================
    /** Replaces each element of this block with the minimum of the corresponding element of the source arrays. */
    template <typename Src1SampleType, typename Src2SampleType>
    SIMDAudioBlock& replaceWithMinOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) noexcept
    {
        replaceWithMinOfInternal (src1, src2);
        return *this;
    }
    template <typename Src1SampleType, typename Src2SampleType>
    const SIMDAudioBlock& replaceWithMinOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        replaceWithMinOfInternal (src1, src2);
        return *this;
    }

    /** Replaces each element of this block with the maximum of the corresponding element of the source arrays. */
    template <typename Src1SampleType, typename Src2SampleType>
    SIMDAudioBlock& replaceWithMaxOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) noexcept
    {
        replaceWithMaxOfInternal (src1, src2);
        return *this;
    }
    template <typename Src1SampleType, typename Src2SampleType>
    const SIMDAudioBlock& replaceWithMaxOf (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        replaceWithMaxOfInternal (src1, src2);
        return *this;
    }

    //==============================================================================
    /** Finds the minimum and maximum value of the buffer. */
    juce::Range<typename std::remove_const<NumericType>::type> findMinAndMax() const noexcept
    {
        if (numChannels == 0)
            return {};

        auto n = numSamples * sizeFactor;
        auto minmax = juce::FloatVectorOperations::findMinAndMax (getDataPointer (0), n);

        for (size_t ch = 1; ch < numChannels; ++ch)
            minmax = minmax.getUnionWith (juce::FloatVectorOperations::findMinAndMax (getDataPointer (ch), n));

        return minmax;
    }

    //==============================================================================
    // Convenient operator wrappers.
    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE operator+= (NumericType value) noexcept { return add (value); }
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE operator+= (NumericType value) const noexcept { return add (value); }

    SIMDAudioBlock& operator+= (SIMDAudioBlock src) noexcept { return add (src); }
    const SIMDAudioBlock& operator+= (SIMDAudioBlock src) const noexcept { return add (src); }

    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE operator-= (NumericType value) noexcept { return subtract (value); }
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE operator-= (NumericType value) const noexcept { return subtract (value); }

    SIMDAudioBlock& operator-= (SIMDAudioBlock src) noexcept { return subtract (src); }
    const SIMDAudioBlock& operator-= (SIMDAudioBlock src) const noexcept { return subtract (src); }

    SIMDAudioBlock& JUCE_VECTOR_CALLTYPE operator*= (NumericType value) noexcept { return multiplyBy (value); }
    const SIMDAudioBlock& JUCE_VECTOR_CALLTYPE operator*= (NumericType value) const noexcept { return multiplyBy (value); }

    SIMDAudioBlock& operator*= (SIMDAudioBlock src) noexcept { return multiplyBy (src); }
    const SIMDAudioBlock& operator*= (SIMDAudioBlock src) const noexcept { return multiplyBy (src); }

    template <typename OtherSampleType, typename SmoothingType>
    SIMDAudioBlock& operator*= (juce::SmoothedValue<OtherSampleType, SmoothingType>& value) noexcept
    {
        return multiplyBy (value);
    }
    template <typename OtherSampleType, typename SmoothingType>
    const SIMDAudioBlock& operator*= (juce::SmoothedValue<OtherSampleType, SmoothingType>& value) const noexcept
    {
        return multiplyBy (value);
    }

    //==============================================================================
    // This class can only be used with floating point types
    static_assert (std::is_same<std::remove_const_t<SampleType>, xsimd::batch<float>>::value
                       || std::is_same<std::remove_const_t<SampleType>, xsimd::batch<double>>::value,
                   "AudioBlock only supports xsimd single or double precision floating point batches");

    //==============================================================================
    /** Applies a function to each value in an input block, putting the result into an output block.
            The function supplied must take a SampleType as its parameter, and return a SampleType.
            The two blocks must have the same number of channels and samples.
        */
    template <typename Src1SampleType, typename Src2SampleType, typename FunctionType>
    static void process (SIMDAudioBlock<Src1SampleType> inBlock, SIMDAudioBlock<Src2SampleType> outBlock, FunctionType&& function)
    {
        auto len = inBlock.getNumSamples();
        auto numChans = inBlock.getNumChannels();

        jassert (len == outBlock.getNumSamples());
        jassert (numChans == outBlock.getNumChannels());

        for (ChannelCountType c = 0; c < numChans; ++c)
        {
            auto* src = inBlock.getChannelPointer (c);
            auto* dst = outBlock.getChannelPointer (c);

            for (size_t i = 0; i < len; ++i)
                dst[i] = function (src[i]);
        }
    }

private:
    NumericType* getDataPointer (size_t channel) const noexcept
    {
        return reinterpret_cast<NumericType*> (getChannelPointer (channel));
    }

    //==============================================================================
    void JUCE_VECTOR_CALLTYPE clearInternal() const noexcept
    {
        auto n = numSamples * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::clear (getDataPointer (ch), n);
    }

    void JUCE_VECTOR_CALLTYPE fillInternal (NumericType value) const noexcept
    {
        auto n = numSamples * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::fill (getDataPointer (ch), value, n);
    }

    template <typename OtherSampleType>
    void copyFromInternal (const SIMDAudioBlock<OtherSampleType>& src) const noexcept
    {
        auto maxChannels = juce::jmin (src.numChannels, numChannels);
        auto n = juce::jmin (src.numSamples * src.sizeFactor, numSamples * sizeFactor);

        for (size_t ch = 0; ch < maxChannels; ++ch)
            juce::FloatVectorOperations::copy (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherNumericType>
    void copyFromInternal (const juce::AudioBuffer<OtherNumericType>& src, size_t srcPos, size_t dstPos, size_t numElements) const
    {
        auto srclen = static_cast<size_t> (src.getNumSamples()) / sizeFactor;
        auto n = juce::jmin (srclen - srcPos, numSamples - dstPos, numElements) * sizeFactor;
        auto maxChannels = juce::jmin (static_cast<size_t> (src.getNumChannels()), static_cast<size_t> (numChannels));

        for (size_t ch = 0; ch < maxChannels; ++ch)
            juce::FloatVectorOperations::copy (getDataPointer (ch) + (dstPos * sizeFactor),
                                               src.getReadPointer ((int) ch, (int) (srcPos * sizeFactor)),
                                               n);
    }

    void moveInternal (size_t srcPos, size_t dstPos, size_t numElements = std::numeric_limits<size_t>::max()) const noexcept
    {
        jassert (srcPos <= numSamples && dstPos <= numSamples);
        auto len = juce::jmin (numSamples - srcPos, numSamples - dstPos, numElements) * sizeof (SampleType);

        if (len != 0)
            for (size_t ch = 0; ch < numChannels; ++ch)
                ::memmove (getChannelPointer (ch) + dstPos,
                           getChannelPointer (ch) + srcPos,
                           len);
    }

    //==============================================================================
    void JUCE_VECTOR_CALLTYPE addInternal (NumericType value) const noexcept
    {
        auto n = numSamples * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::add (getDataPointer (ch), value, n);
    }

    template <typename OtherSampleType>
    void addInternal (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = juce::jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::add (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherSampleType>
    void JUCE_VECTOR_CALLTYPE replaceWithSumOfInternal (SIMDAudioBlock<OtherSampleType> src, NumericType value) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = juce::jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::add (getDataPointer (ch), src.getDataPointer (ch), value, n);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithSumOfInternal (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = juce::jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::add (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    //==============================================================================
    constexpr void JUCE_VECTOR_CALLTYPE subtractInternal (NumericType value) const noexcept
    {
        addInternal (value * static_cast<NumericType> (-1.0));
    }

    template <typename OtherSampleType>
    void subtractInternal (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = juce::jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::subtract (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherSampleType>
    void JUCE_VECTOR_CALLTYPE replaceWithDifferenceOfInternal (SIMDAudioBlock<OtherSampleType> src, NumericType value) const noexcept
    {
        replaceWithSumOfInternal (src, static_cast<NumericType> (-1.0) * value);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithDifferenceOfInternal (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = juce::jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::subtract (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    //==============================================================================
    void JUCE_VECTOR_CALLTYPE multiplyByInternal (NumericType value) const noexcept
    {
        auto n = numSamples * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::multiply (getDataPointer (ch), value, n);
    }

    template <typename OtherSampleType>
    void multiplyByInternal (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = juce::jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::multiply (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherSampleType>
    void JUCE_VECTOR_CALLTYPE replaceWithProductOfInternal (SIMDAudioBlock<OtherSampleType> src, NumericType value) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = juce::jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::multiply (getDataPointer (ch), src.getDataPointer (ch), value, n);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithProductOfInternal (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = juce::jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::multiply (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    template <typename OtherSampleType, typename SmoothingType>
    void multiplyByInternal (juce::SmoothedValue<OtherSampleType, SmoothingType>& value) const noexcept
    {
        if (! value.isSmoothing())
        {
            multiplyByInternal ((NumericType) value.getTargetValue());
        }
        else
        {
            for (size_t i = 0; i < numSamples; ++i)
            {
                const auto scaler = (NumericType) value.getNextValue();

                for (size_t ch = 0; ch < numChannels; ++ch)
                    getDataPointer (ch)[i] *= scaler;
            }
        }
    }

    template <typename BlockSampleType, typename SmootherSampleType, typename SmoothingType>
    void replaceWithProductOfInternal (SIMDAudioBlock<BlockSampleType> src, juce::SmoothedValue<SmootherSampleType, SmoothingType>& value) const noexcept
    {
        jassert (numChannels == src.numChannels);

        if (! value.isSmoothing())
        {
            replaceWithProductOfInternal (src, (NumericType) value.getTargetValue());
        }
        else
        {
            auto n = juce::jmin (numSamples, src.numSamples) * sizeFactor;

            for (size_t i = 0; i < n; ++i)
            {
                const auto scaler = (NumericType) value.getNextValue();

                for (size_t ch = 0; ch < numChannels; ++ch)
                    getDataPointer (ch)[i] = scaler * src.getChannelPointer (ch)[i];
            }
        }
    }

    //==============================================================================
    template <typename OtherSampleType>
    void JUCE_VECTOR_CALLTYPE addProductOfInternal (SIMDAudioBlock<OtherSampleType> src, NumericType factor) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = juce::jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::addWithMultiply (getDataPointer (ch), src.getDataPointer (ch), factor, n);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void addProductOfInternal (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = juce::jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::addWithMultiply (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    //==============================================================================
    constexpr void negateInternal() const noexcept
    {
        multiplyByInternal (static_cast<NumericType> (-1.0));
    }

    template <typename OtherSampleType>
    void replaceWithNegativeOfInternal (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = juce::jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::negate (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherSampleType>
    void replaceWithAbsoluteValueOfInternal (SIMDAudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = juce::jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::abs (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    //==============================================================================
    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithMinOfInternal (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = juce::jmin (src1.numSamples, src2.numSamples, numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::min (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithMaxOfInternal (SIMDAudioBlock<Src1SampleType> src1, SIMDAudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = juce::jmin (src1.numSamples, src2.numSamples, numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::max (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    //==============================================================================
    using ChannelCountType = unsigned int;

    //==============================================================================
    static constexpr size_t sizeFactor = sizeof (SampleType) / sizeof (NumericType);
    static constexpr size_t elementMask = sizeFactor - 1;
    static constexpr size_t byteMask = (sizeFactor * sizeof (NumericType)) - 1;

    static constexpr size_t defaultAlignment = sizeof (xsimd::batch<std::remove_const_t<NumericType>>);

    SampleType* const* channels;
    ChannelCountType numChannels = 0;
    size_t startSample = 0, numSamples = 0;

    template <typename OtherSampleType>
    friend class SIMDAudioBlock;
};

/** Wrapper of juce::dsp::AudioBlock, that allows AudioBlock<xsimd::batch<T>> to work. */
template <typename T>
using AudioBlock = std::conditional_t<SampleTypeHelpers::IsSIMDRegister<std::remove_const_t<T>>, SIMDAudioBlock<T>, juce::dsp::AudioBlock<T>>;

//==============================================================================
/**
    Contains context information that is passed into an algorithm's process method.

    This context is intended for use in situations where a single block is being used
    for both the input and output, so it will return the same object for both its
    getInputBlock() and getOutputBlock() methods.

    @see ProcessContextNonReplacing
*/
template <typename ContextSampleType>
struct ProcessContextReplacing
{
public:
    /** The type of a single sample (which may be a vector if multichannel). */
    using SampleType = ContextSampleType;
    /** The type of audio block that this context handles. */
    using AudioBlockType = AudioBlock<SampleType>;
    using ConstAudioBlockType = AudioBlock<const SampleType>;

    /** Creates a ProcessContextReplacing that uses the given audio block.
        Note that the caller must not delete the block while it is still in use by this object!
    */
    explicit ProcessContextReplacing (AudioBlockType& block) noexcept : ioBlock (block) {}

    ProcessContextReplacing (const ProcessContextReplacing&) = default;
    ProcessContextReplacing (ProcessContextReplacing&&) noexcept = default;

    /** Returns the audio block to use as the input to a process function. */
    const ConstAudioBlockType& getInputBlock() const noexcept { return constBlock; }

    /** Returns the audio block to use as the output to a process function. */
    AudioBlockType& getOutputBlock() const noexcept { return ioBlock; }

    /** All process context classes will define this constant method so that templated
        code can determine whether the input and output blocks refer to the same buffer,
        or to two different ones.
    */
    static constexpr bool usesSeparateInputAndOutputBlocks() { return false; }

    /** If set to true, then a processor's process() method is expected to do whatever
        is appropriate for it to be in a bypassed state.
    */
    bool isBypassed = false;

private:
    AudioBlockType& ioBlock;
    ConstAudioBlockType constBlock { ioBlock };
};

//==============================================================================
/**
    Contains context information that is passed into an algorithm's process method.

    This context is intended for use in situations where two different blocks are being
    used the input and output to the process algorithm, so the processor must read from
    the block returned by getInputBlock() and write its results to the block returned by
    getOutputBlock().

    @see ProcessContextReplacing
*/
template <typename ContextSampleType>
struct ProcessContextNonReplacing
{
public:
    /** The type of a single sample (which may be a vector if multichannel). */
    using SampleType = ContextSampleType;
    /** The type of audio block that this context handles. */
    using AudioBlockType = AudioBlock<SampleType>;
    using ConstAudioBlockType = AudioBlock<const SampleType>;

    /** Creates a ProcessContextNonReplacing that uses the given input and output blocks.
        Note that the caller must not delete these blocks while they are still in use by this object!
    */
    ProcessContextNonReplacing (const ConstAudioBlockType& input, AudioBlockType& output) noexcept
        : inputBlock (input), outputBlock (output)
    {
        // If the input and output blocks are the same then you should use
        // ProcessContextReplacing instead.
        jassert (input != output);
    }

    ProcessContextNonReplacing (const ProcessContextNonReplacing&) = default;
    ProcessContextNonReplacing (ProcessContextNonReplacing&&) noexcept = default;

    /** Returns the audio block to use as the input to a process function. */
    const ConstAudioBlockType& getInputBlock() const noexcept { return inputBlock; }

    /** Returns the audio block to use as the output to a process function. */
    AudioBlockType& getOutputBlock() const noexcept { return outputBlock; }

    /** All process context classes will define this constant method so that templated
        code can determine whether the input and output blocks refer to the same buffer,
        or to two different ones.
    */
    static constexpr bool usesSeparateInputAndOutputBlocks() { return true; }

    /** If set to true, then a processor's process() method is expected to do whatever
        is appropriate for it to be in a bypassed state.
    */
    bool isBypassed = false;

private:
    ConstAudioBlockType inputBlock;
    AudioBlockType& outputBlock;
};
} // namespace chowdsp
