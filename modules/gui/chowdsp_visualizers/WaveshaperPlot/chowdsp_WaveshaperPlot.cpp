namespace chowdsp
{
WaveshaperPlot::WaveshaperPlot (WaveshaperPlotParams&& plotParams) : params (std::move (plotParams)) {}

float WaveshaperPlot::getXCoordinateForAmplitude (float amplitude) const noexcept
{
    amplitude = juce::jlimit (params.xMin, params.xMax, amplitude);
    return juce::jmap (amplitude, params.xMin, params.xMax, 0.0f, (float) bounds.getWidth());
}

float WaveshaperPlot::getYCoordinateForAmplitude (float amplitude) const noexcept
{
    amplitude = juce::jlimit (params.yMin, params.yMax, amplitude);
    return juce::jmap (amplitude, params.yMin, params.yMax, (float) bounds.getHeight(), 0.0f);
}

void WaveshaperPlot::updatePlotPath()
{
    jassert (generatePlotCallback != nullptr);
    auto [xBuffer, yBuffer] = generatePlotCallback();

    plotPath.clear();
    if (xBuffer.empty() || yBuffer.empty())
    {
        return;
    }

    jassert (xBuffer.size() == yBuffer.size());
    plotPath.preallocateSpace (3 * (int) xBuffer.size());

    const auto getPoint = [this] (float xAmp, float yAmp) -> juce::Point<float>
    {
        return { getXCoordinateForAmplitude (xAmp), getYCoordinateForAmplitude (yAmp) };
    };

    bool pathStarted = false;
    for (auto [xAmp, yAmp] : chowdsp::zip (xBuffer, yBuffer))
    {
        if (! pathStarted)
        {
            plotPath.startNewSubPath (getPoint (xAmp, yAmp));
            pathStarted = true;
        }
        else
        {
            plotPath.lineTo (getPoint (xAmp, yAmp));
        }
    }
}

void WaveshaperPlot::setSize (juce::Rectangle<int> newBounds)
{
    bounds = newBounds;
    updatePlotPath();
}
} // namespace chowdsp
