namespace chowdsp
{
GenericFilterPlotter::GenericFilterPlotter (const SpectrumPlotBase& plotBase, Params&& plotParams)
    : params (std::move (plotParams)),
      base (plotBase),
      fft (params.fftOrder),
      fftSize (1 << params.fftOrder),
      sweepBuffer (generateLogSweep (fftSize, params.sampleRate, base.params.minFrequencyHz, base.params.maxFrequencyHz)),
      freqAxis (fftFreqs (fftSize / 2 + 1, 1.0f / params.sampleRate))
{
    filterBuffer = std::vector<float> ((size_t) fftSize, 0.0f);
    sweepFFT = std::vector<float> ((size_t) fftSize * 2, 0.0f);
    filtFFT = std::vector<float> ((size_t) fftSize * 2, 0.0f);

    const auto fftOutSize = fftSize / 2 + 1;
    magResponseDB = std::vector<float> ((size_t) fftOutSize, 0.0f);
    magResponseSmoothDB = std::vector<float> ((size_t) fftOutSize, 0.0f);
}

std::pair<const std::vector<float>&, const std::vector<float>&> GenericFilterPlotter::plotFilterMagnitudeResponse()
{
    std::fill (filterBuffer.begin(), filterBuffer.end(), 0.0f);
    runFilterCallback (sweepBuffer.data(), filterBuffer.data(), fftSize);

    computeFrequencyResponse();
    freqSmooth (magResponseDB, magResponseSmoothDB, params.freqSmoothOctaves);

    return { freqAxis, magResponseSmoothDB };
}

void GenericFilterPlotter::updateFilterPlot()
{
    const auto [_, magResponseDBSmoothed] = plotFilterMagnitudeResponse();

    const juce::ScopedLock pathLock { pathMutex };
    plotPath.clear();
    bool started = false;
    for (size_t i = 0; i < freqAxis.size(); ++i)
    {
        if (freqAxis[i] < base.params.minFrequencyHz / 2.0f || freqAxis[i] > base.params.maxFrequencyHz * 1.01f)
            continue;

        auto xDraw = base.getXCoordinateForFrequency (freqAxis[i]);
        auto yDraw = base.getYCoordinateForDecibels (magResponseDBSmoothed[i]);

        if (! started)
        {
            plotPath.startNewSubPath (xDraw, yDraw);
            started = true;
        }
        else
        {
            plotPath.lineTo (xDraw, yDraw);
        }
    }
}

std::vector<float> GenericFilterPlotter::generateLogSweep (int nSamples, float sampleRate, float startFreqHz, float endFreqHz)
{
    std::vector<float> sweepBuffer ((size_t) nSamples, 0.0f);
    const auto beta = (float) nSamples / std::log (endFreqHz / startFreqHz);

    for (size_t i = 0; i < (size_t) nSamples; i++)
    {
        float phase = juce::MathConstants<float>::twoPi * beta * startFreqHz * (std::pow (endFreqHz / startFreqHz, (float) i / (float) nSamples) - 1.0f);
        sweepBuffer[i] = std::sin ((phase + juce::MathConstants<float>::pi / 180.0f) / sampleRate);
    }
    return sweepBuffer;
}

void GenericFilterPlotter::computeFrequencyResponse()
{
    std::copy (sweepBuffer.begin(), sweepBuffer.begin() + fftSize, sweepFFT.begin());
    fft.performFrequencyOnlyForwardTransform (sweepFFT.data(), true);

    std::copy (filterBuffer.begin(), filterBuffer.begin() + fftSize, filtFFT.begin());
    fft.performFrequencyOnlyForwardTransform (filtFFT.data(), true);

    const auto fftOutSize = fftSize / 2 + 1;
    for (size_t i = 0; i < (size_t) fftOutSize; ++i)
        magResponseDB[i] = juce::Decibels::gainToDecibels (filtFFT[i] / sweepFFT[i]);
}

std::vector<float> GenericFilterPlotter::fftFreqs (int N, float T)
{
    auto val = 0.5f / ((float) N * T);

    std::vector<float> results ((size_t) N, 0.0f);
    std::iota (results.begin(), results.end(), 0.0f);
    std::transform (results.begin(), results.end(), results.begin(), [val] (auto x)
                    { return x * val; });

    return results;
}

void GenericFilterPlotter::freqSmooth (const std::vector<float>& magsDB, std::vector<float>& magsSmoothDB, float smFactor) // (const float* data, int numSamples, float smFactor)
{
    const auto s = smFactor > 1.0f ? smFactor : std::sqrt (std::pow (2.0f, smFactor));

    const auto numSamples = magsDB.size();
    for (size_t i = 0; i < numSamples; ++i)
    {
        auto i1 = std::max (int ((float) i / s), 0);
        auto i2 = std::min (int ((float) i * s) + 1, (int) numSamples - 1);
        magsSmoothDB[i] = i2 > i1 ? std::accumulate (magsDB.data() + i1, magsDB.data() + i2, 0.0f) / float (i2 - i1) : 0.0f;
    }
}
} // namespace chowdsp
