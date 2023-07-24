namespace chowdsp
{
template <int numChannels>
WaveformView<numChannels>::ChannelInfo::ChannelInfo (WaveformView& o, int bufferSize) : owner (o)
{
    setBufferSize (bufferSize);
    clear();
}

template <int numChannels>
void WaveformView<numChannels>::ChannelInfo::clear() noexcept
{
    std::fill (levels.begin(), levels.end(), juce::Range<float> {});
    value = {};
    subSample = 0;
}

template <int numChannels>
template <typename T>
void WaveformView<numChannels>::ChannelInfo::pushSamples (const T* inputSamples, int num) noexcept
{
    for (int i = 0; i < num; ++i)
        pushSample ((float) inputSamples[i]);
}

template <int numChannels>
void WaveformView<numChannels>::ChannelInfo::pushSample (float newSample) noexcept
{
    if (--subSample <= 0)
    {
        if (++nextSample == (int) levels.size())
            nextSample = 0;

        levels[(size_t) nextSample.load()] = value;
        subSample = owner.getSamplesPerBlock();
        value = juce::Range<float> (newSample, newSample);
    }
    else
    {
        value = value.getUnionWith (newSample);
    }
}

template <int numChannels>
void WaveformView<numChannels>::ChannelInfo::setBufferSize (int newSize)
{
    levels.resize ((size_t) newSize, juce::Range<float> {});
    if (nextSample >= newSize)
        nextSample = 0;
}

//============================================
template <int numChannels>
WaveformView<numChannels>::WaveformView()
{
    startTimerHz (30);
    setOpaque (false);
}

template <int numChannels>
void WaveformView<numChannels>::visibilityChanged()
{
    if (isVisible())
    {
        startTimerHz (30);
    }
    else
    {
        clear();
        stopTimer();
    }
}

template <int numChannels>
void WaveformView<numChannels>::setBufferSize (int newNumSamples)
{
    numSamples = newNumSamples;
    for (auto& c : channels)
        c.setBufferSize (newNumSamples);
}

template <int numChannels>
void WaveformView<numChannels>::setSamplesPerBlock (int newSamplesPerPixel) noexcept
{
    inputSamplesPerBlock = newSamplesPerPixel;
}

template <int numChannels>
void WaveformView<numChannels>::clear()
{
    for (auto& c : channels)
        c.clear();
}

template <int numChannels>
template <typename SampleType>
void WaveformView<numChannels>::pushBuffer (const BufferView<const SampleType>& buffer) noexcept
{
    jassert (buffer.getNumChannels() == (int) channels.size());
    for (auto [ch, channelData] : buffer_iters::channels (buffer))
        pushChannel ((int) ch, channelData);
}

template <int numChannels>
template <typename SampleType>
void WaveformView<numChannels>::pushChannel (int channelIndex, const nonstd::span<const SampleType>& channelData) noexcept
{
    channels[(size_t) channelIndex].pushSamples (channelData.data(), (int) channelData.size());
}

template <int numChannels>
void WaveformView<numChannels>::paint (juce::Graphics& g)
{
    g.fillAll (backgroundColour);

    auto bounds = getLocalBounds().toFloat();
    for (auto [ch, c] : chowdsp::enumerate (channels))
        paintChannel ((int) ch, g, bounds, c.levels.data(), (int) c.levels.size(), c.nextSample);
}

template <int numChannels>
void WaveformView<numChannels>::paintChannel (int /*channelIndex*/, juce::Graphics& g, juce::Rectangle<float> area, const juce::Range<float>* levels, int numLevels, int nextSample)
{
    g.setColour (juce::Colours::black);

    juce::Path path;
    path.preallocateSpace (4 * numLevels + 8);

    for (int i = 0; i < numLevels; ++i)
    {
        auto level = -(levels[(nextSample + i) % numLevels].getEnd());

        if (i == 0)
            path.startNewSubPath (0.0f, level);
        else
            path.lineTo ((float) i, level);
    }

    for (int i = numLevels; --i >= 0;)
        path.lineTo ((float) i, -(levels[(nextSample + i) % numLevels].getStart()));

    path.closeSubPath();

    g.fillPath (path,
                juce::AffineTransform::fromTargetPoints (0.0f,
                                                         -1.0f,
                                                         area.getX(),
                                                         area.getY(),
                                                         0.0f,
                                                         1.0f,
                                                         area.getX(),
                                                         area.getBottom(),
                                                         (float) numLevels,
                                                         -1.0f,
                                                         area.getRight(),
                                                         area.getY()));
}
} // namespace chowdsp
