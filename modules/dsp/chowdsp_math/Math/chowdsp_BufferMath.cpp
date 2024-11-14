namespace chowdsp::BufferMath
{
template <typename BufferType>
auto getMagnitude (const BufferType& buffer, int startSample, int numSamples, int channel) noexcept
{
    if (numSamples < 0)
        numSamples = buffer.getNumSamples() - startSample;

    using SampleType = BufferSampleType<BufferType>;
    auto getChannelMagnitude = [&buffer, startSample, numSamples] (int ch)
    {
        const auto* channelData = buffer.getReadPointer (ch);
        if constexpr (std::is_floating_point_v<SampleType>)
        {
            return FloatVectorOperations::findAbsoluteMaximum (channelData + startSample, numSamples);
        }
#if ! CHOWDSP_NO_XSIMD
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            return std::accumulate (channelData + startSample,
                                    channelData + startSample + numSamples,
                                    SampleType {},
                                    [] (const auto& prev, const auto& next)
                                    {
                                        return xsimd::max (prev, xsimd::abs (next));
                                    });
        }
#endif
    };

    if (channel >= 0)
        return getChannelMagnitude (channel);

    CHOWDSP_USING_XSIMD_STD (max);

    SampleType mag {};
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        mag = max (getChannelMagnitude (ch), mag);
    return mag;
}

template <typename BufferType>
auto getRMSLevel (const BufferType& buffer, int channel, int startSample, int numSamples) noexcept
{
    using SampleType = BufferSampleType<BufferType>;

    if (numSamples < 0)
        numSamples = buffer.getNumSamples() - startSample;

    if (numSamples <= 0 || channel < 0 || channel >= buffer.getNumChannels())
        return SampleType (0);

    auto* data = buffer.getReadPointer (channel) + startSample;
    if constexpr (std::is_floating_point_v<SampleType>)
    {
        return FloatVectorOperations::computeRMS (data, numSamples);
    }
#if ! CHOWDSP_NO_XSIMD
    else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
    {
        using NumericType = SampleTypeHelpers::NumericType<SampleType>;

        auto sum = SampleType (0);
        for (int i = 0; i < numSamples; ++i)
        {
            auto sample = data[i];
            sum += sample * sample;
        }

        return xsimd::sqrt (sum / (NumericType) numSamples);
    }
#endif
}

template <typename BufferType1, typename BufferType2>
void copyBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample, int destStartSample, int numSamples, int startChannel, int numChannels) noexcept
{
    using SampleType1 = BufferSampleType<BufferType1>;
    using SampleType2 = BufferSampleType<BufferType2>;
    static_assert (std::is_same_v<SampleType1, SampleType2> || (std::is_floating_point_v<SampleType1> && std::is_floating_point_v<SampleType2>),
                   "Both buffer types must have the same sample type!");

    if (numSamples < 0)
    {
        jassert (bufferSrc.getNumSamples() == bufferDest.getNumSamples());
        numSamples = bufferDest.getNumSamples();
    }

    if (numChannels < 0)
    {
        jassert (bufferSrc.getNumChannels() == bufferDest.getNumChannels());
        numChannels = bufferDest.getNumChannels();
    }

    jassert (srcStartSample + numSamples <= bufferSrc.getNumSamples());
    jassert (destStartSample + numSamples <= bufferDest.getNumSamples());

    for (int ch = startChannel; ch < startChannel + numChannels; ++ch)
    {
        const auto* srcData = bufferSrc.getReadPointer (ch);
        auto* destData = bufferDest.getWritePointer (ch);

        // If you're here, check that you're calling this function correctly,
        // the channel is probably out of bounds.
        jassert (destData != nullptr);
        jassert (srcData != nullptr);

        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4244)
        std::copy (srcData + srcStartSample, srcData + srcStartSample + numSamples, destData + destStartSample);
        JUCE_END_IGNORE_WARNINGS_MSVC
    }
}

template <typename BufferType1, typename BufferType2>
void copyBufferChannels (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcChannel, int destChannel) noexcept
{
    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    jassert (bufferSrc.getNumSamples() == bufferDest.getNumSamples());
    const auto numSamples = bufferSrc.getNumSamples();

    const auto* srcData = bufferSrc.getReadPointer (srcChannel);
    auto* destData = bufferDest.getWritePointer (destChannel);

    if constexpr (std::is_floating_point_v<SampleType>)
    {
        juce::FloatVectorOperations::copy (destData, srcData, numSamples);
    }
    else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
    {
        std::copy (srcData, srcData + numSamples, destData);
    }
}

template <typename BufferType1, typename BufferType2>
void addBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample, int destStartSample, int numSamples, int startChannel, int numChannels) noexcept
{
    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    if (numSamples < 0)
    {
        jassert (bufferSrc.getNumSamples() == bufferDest.getNumSamples());
        numSamples = bufferDest.getNumSamples();
    }

    if (numChannels < 0)
    {
        jassert (bufferSrc.getNumChannels() == bufferDest.getNumChannels());
        numChannels = bufferDest.getNumChannels();
    }

    jassert (srcStartSample + numSamples <= bufferSrc.getNumSamples());
    jassert (destStartSample + numSamples <= bufferDest.getNumSamples());

    for (int ch = startChannel; ch < startChannel + numChannels; ++ch)
    {
        const auto* srcData = bufferSrc.getReadPointer (ch);
        auto* destData = bufferDest.getWritePointer (ch);

        if constexpr (std::is_floating_point_v<SampleType>)
        {
            juce::FloatVectorOperations::add (destData + destStartSample, srcData + srcStartSample, numSamples);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            std::transform (srcData + srcStartSample,
                            srcData + srcStartSample + numSamples,
                            destData + destStartSample,
                            destData + destStartSample,
                            [] (const auto& a, const auto& b)
                            { return a + b; });
        }
    }
}

/** Adds channels from one buffer into another. */
template <typename BufferType1, typename BufferType2>
void addBufferChannels (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcChannel, int destChannel) noexcept
{
    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    jassert (bufferSrc.getNumSamples() == bufferDest.getNumSamples());
    const auto numSamples = bufferSrc.getNumSamples();

    const auto* srcData = bufferSrc.getReadPointer (srcChannel);
    auto* destData = bufferDest.getWritePointer (destChannel);

    if constexpr (std::is_floating_point_v<SampleType>)
    {
        juce::FloatVectorOperations::add (destData, srcData, numSamples);
    }
    else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
    {
        std::transform (srcData,
                        srcData + numSamples,
                        destData,
                        destData,
                        [] (const auto& a, const auto& b)
                        { return a + b; });
    }
}

template <typename BufferType1, typename BufferType2>
void multiplyBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample, int destStartSample, int numSamples, int startChannel, int numChannels) noexcept
{
    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    if (numSamples < 0)
    {
        jassert (bufferSrc.getNumSamples() == bufferDest.getNumSamples());
        numSamples = bufferDest.getNumSamples();
    }

    if (numChannels < 0)
    {
        jassert (bufferSrc.getNumChannels() == bufferDest.getNumChannels());
        numChannels = bufferDest.getNumChannels();
    }

    jassert (srcStartSample + numSamples <= bufferSrc.getNumSamples());
    jassert (destStartSample + numSamples <= bufferDest.getNumSamples());

    for (int ch = startChannel; ch < startChannel + numChannels; ++ch)
    {
        const auto* srcData = bufferSrc.getReadPointer (ch);
        auto* destData = bufferDest.getWritePointer (ch);

        if constexpr (std::is_floating_point_v<SampleType>)
        {
            juce::FloatVectorOperations::multiply (destData + destStartSample, srcData + srcStartSample, numSamples);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            std::transform (srcData + srcStartSample,
                            srcData + srcStartSample + numSamples,
                            destData + destStartSample,
                            destData + destStartSample,
                            [] (const auto& a, const auto& b)
                            { return a * b; });
        }
    }
}

template <typename BufferType, typename FloatType>
void applyGain (BufferType& buffer, FloatType gain) noexcept
{
    applyGain (buffer, buffer, gain);
}

template <typename BufferType1, typename BufferType2, typename FloatType>
void applyGain (const BufferType1& bufferSrc, BufferType2& bufferDest, FloatType gain) noexcept
{
    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    const auto numChannels = bufferSrc.getNumChannels();
    const auto numSamples = bufferSrc.getNumSamples();

    // both buffers must have the same size
    jassert (bufferDest.getNumChannels() == numChannels);
    jassert (bufferDest.getNumSamples() == numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const auto* dataIn = bufferSrc.getReadPointer (ch);
        auto* dataOut = bufferDest.getWritePointer (ch);

        if constexpr (std::is_floating_point_v<SampleType>)
        {
            juce::FloatVectorOperations::multiply (dataOut, dataIn, gain, numSamples);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            std::transform (dataIn, dataIn + numSamples, dataOut, [gain] (const auto& x)
                            { return x * gain; });
        }
    }
}

template <typename BufferType, typename SmoothedValueType>
void applyGainSmoothed (BufferType& buffer, SmoothedValueType& gain) noexcept
{
    applyGainSmoothed (buffer, buffer, gain);
}

template <typename BufferType1, typename SmoothedValueType, typename BufferType2>
void applyGainSmoothed (const BufferType1& bufferSrc, BufferType2& bufferDest, SmoothedValueType& gain) noexcept
{
    if (! gain.isSmoothing())
    {
        applyGain (bufferSrc, bufferDest, gain.getCurrentValue());
        return;
    }

    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    const auto numChannels = bufferSrc.getNumChannels();
    const auto numSamples = bufferSrc.getNumSamples();

    // both buffers must have the same size
    jassert (bufferDest.getNumChannels() == numChannels);
    jassert (bufferDest.getNumSamples() == numSamples);

    const auto dataIn = bufferSrc.getArrayOfReadPointers();
    auto dataOut = bufferDest.getArrayOfWritePointers();
    for (int n = 0; n < numSamples; ++n)
    {
        const auto sampleGain = gain.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
            dataOut[ch][n] = dataIn[ch][n] * sampleGain;
    }
}

template <typename BufferType, typename SmoothedBufferType>
void applyGainSmoothedBuffer (BufferType& buffer, SmoothedBufferType& gain) noexcept
{
    applyGainSmoothedBuffer (buffer, buffer, gain);
}

template <typename BufferType1, typename SmoothedBufferType, typename BufferType2>
void applyGainSmoothedBuffer (const BufferType1& bufferSrc, BufferType2& bufferDest, SmoothedBufferType& gain) noexcept
{
    if (! gain.isSmoothing())
    {
        applyGain (bufferSrc, bufferDest, gain.getCurrentValue());
        return;
    }

    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    const auto numChannels = bufferSrc.getNumChannels();
    const auto numSamples = bufferSrc.getNumSamples();

    // both buffers must have the same size
    jassert (bufferDest.getNumChannels() == numChannels);
    jassert (bufferDest.getNumSamples() == numSamples);

    const auto audioDataIn = bufferDest.getArrayOfReadPointers();
    auto audioDataOut = bufferDest.getArrayOfWritePointers();
    const auto gainData = gain.getSmoothedBuffer();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        if constexpr (std::is_floating_point_v<SampleType>)
            juce::FloatVectorOperations::multiply (audioDataOut[ch], audioDataIn[ch], gainData, numSamples);
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
            std::transform (audioDataIn[ch], audioDataIn[ch] + numSamples, gainData, audioDataOut[ch], [] (const auto& x, const auto& g)
                            { return x * g; });
    }
}

template <typename BufferType1, typename BufferType2, typename FloatType>
void sumToMono (const BufferType1& bufferSrc, BufferType2& bufferDest, FloatType normGain)
{
    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    const auto numInChannels = bufferSrc.getNumChannels();
    [[maybe_unused]] const auto numSamples = bufferSrc.getNumSamples();

    // both buffers must have the same size
    jassert (bufferDest.getNumSamples() == numSamples);

    if (normGain < (FloatType) 0)
        normGain = (FloatType) 1 / (FloatType) bufferSrc.getNumChannels();

    if (bufferSrc.getReadPointer (0) != bufferDest.getWritePointer (0))
        copyBufferChannels (bufferSrc, bufferDest, 0, 0);

    for (int ch = 1; ch < numInChannels; ++ch)
        addBufferChannels (bufferSrc, bufferDest, ch, 0);

    // apply normalization gain
    if (! juce::exactlyEqual (normGain, (FloatType) 1))
    {
        if constexpr (std::is_floating_point_v<SampleType>)
        {
            juce::FloatVectorOperations::multiply (bufferDest.getWritePointer (0),
                                                   bufferDest.getReadPointer (0),
                                                   normGain,
                                                   numSamples);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            std::transform (bufferDest.getReadPointer (0),
                            bufferDest.getReadPointer (0) + numSamples,
                            bufferDest.getWritePointer (0),
                            [normGain] (const auto& x)
                            { return x * normGain; });
        }
    }
}

template <typename BufferType, typename FloatType>
bool sanitizeBuffer (BufferType& buffer, FloatType ceiling) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    bool needsClear = false;
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const auto* channelData = buffer.getReadPointer (ch);
        const auto channelMax = FloatVectorOperations::findAbsoluteMaximum (channelData, numSamples);
        const auto channelInfsAndNaNs = FloatVectorOperations::countInfsAndNaNs (channelData, numSamples);

        if (channelMax >= ceiling || channelInfsAndNaNs > 0)
        {
            // This buffer contains invalid values! Clearing...
            jassertfalse;
            needsClear = true;
        }
    }

    if (needsClear)
        buffer.clear();

    return ! needsClear;
}

template <typename BufferType, typename FunctionType>
void applyFunction (BufferType& buffer, FunctionType&& function) noexcept
{
    applyFunction (buffer, buffer, std::forward<FunctionType> (function));
}

template <typename BufferType1, typename BufferType2, typename FunctionType>
void applyFunction (const BufferType1& bufferSrc, BufferType2& bufferDest, FunctionType&& function) noexcept
{
    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    const auto numChannels = bufferSrc.getNumChannels();
    const auto numSamples = bufferSrc.getNumSamples();

    // both buffers must have the same size
    jassert (bufferDest.getNumChannels() == numChannels);
    jassert (bufferDest.getNumSamples() == numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const auto* dataIn = bufferSrc.getReadPointer (ch);
        auto* dataOut = bufferDest.getWritePointer (ch);
        for (int n = 0; n < numSamples; ++n)
            dataOut[n] = function (dataIn[n]);
    }
}

#if ! CHOWDSP_NO_XSIMD
template <typename BufferType, typename FunctionType, typename FloatType>
std::enable_if_t<std::is_floating_point_v<FloatType>, void> applyFunctionSIMD (BufferType& buffer, FunctionType&& function) noexcept
{
    applyFunctionSIMD (buffer, std::forward<FunctionType> (function), std::forward<FunctionType> (function));
}

template <typename BufferType1, typename BufferType2, typename FunctionType, typename FloatType>
std::enable_if_t<std::is_floating_point_v<FloatType>, void>
    applyFunctionSIMD (const BufferType1& bufferSrc, BufferType2& bufferDest, FunctionType&& function) noexcept
{
    applyFunctionSIMD (bufferSrc, bufferDest, std::forward<FunctionType> (function), std::forward<FunctionType> (function));
}

template <typename BufferType, typename SIMDFunctionType, typename ScalarFunctionType, typename FloatType>
std::enable_if_t<std::is_floating_point_v<FloatType>, void>
    applyFunctionSIMD (BufferType& buffer, SIMDFunctionType&& simdFunction, ScalarFunctionType&& scalarFunction) noexcept
{
    applyFunctionSIMD (buffer, buffer, std::forward<SIMDFunctionType> (simdFunction), std::forward<ScalarFunctionType> (scalarFunction));
}

template <typename BufferType1, typename BufferType2, typename SIMDFunctionType, typename ScalarFunctionType, typename FloatType>
std::enable_if_t<std::is_floating_point_v<FloatType>, void>
    applyFunctionSIMD (const BufferType1& bufferSrc, BufferType2& bufferDest, SIMDFunctionType&& simdFunction, ScalarFunctionType&& scalarFunction) noexcept
{
#if CHOWDSP_USING_JUCE
    static_assert (! std::is_same_v<BufferType1, juce::AudioBuffer<FloatType>>,
                   "applyFunctionSIMD expects that the buffers channel data will be aligned"
                   "which cannot be guaranteed with juce::AudioBuffer!");
    static_assert (! std::is_same_v<BufferType2, juce::AudioBuffer<FloatType>>,
                   "applyFunctionSIMD expects that the buffers channel data will be aligned"
                   "which cannot be guaranteed with juce::AudioBuffer!");
#endif

    using SampleType = BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    const auto numChannels = bufferSrc.getNumChannels();
    const auto numSamples = bufferSrc.getNumSamples();

    // both buffers must have the same size
    jassert (bufferDest.getNumChannels() == numChannels);
    jassert (bufferDest.getNumSamples() == numSamples);

#if JUCE_DEBUG
    for (int ch = 0; ch < numChannels; ++ch)
    {
        jassert (SIMDUtils::isAligned (bufferSrc.getReadPointer (ch)));
        jassert (SIMDUtils::isAligned (bufferDest.getReadPointer (ch)));
    }
#endif

    static constexpr auto vecSize = (int) xsimd::batch<FloatType>::size;
    const auto numVecOps = numSamples / vecSize;
    const auto leftoverValues = numSamples % vecSize;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const auto* dataIn = bufferSrc.getReadPointer (ch);
        auto* dataOut = bufferDest.getWritePointer (ch);

        auto channelVecOps = numVecOps;
        while (--channelVecOps >= 0)
        {
            xsimd::store_aligned (dataOut, simdFunction (xsimd::load_aligned (dataIn)));
            dataIn += vecSize;
            dataOut += vecSize;
        }

        for (int n = 0; n < leftoverValues; ++n)
            dataOut[n] = scalarFunction (dataIn[n]);
    }
}
#endif // ! CHOWDSP_NO_XSIMD
} // namespace chowdsp::BufferMath
