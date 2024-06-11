#pragma once

namespace chowdsp
{
/**
 * Helper class for plotting the frequency response of some generic filter.
 *
 * The class generates the plot as a juce::Path, by passing a sine sweep
 * through the filter (supplied by the user via a std::function), and then
 * computing the frequency response with partial-octave-band smoothing.
 */
class GenericFilterPlotter
{
public:
    /** Parameters for the plot. */
    struct Params
    {
        float sampleRate = 48000.0f;
        float freqSmoothOctaves = 1.0f / 12.0f;
        int fftOrder = 13;
    };

    /** Constructs a plotter from a SpectrumPlotBase */
    explicit GenericFilterPlotter (const SpectrumPlotBase& plotBase, Params&& plotParams);

    /** Runs data through the filter process and returns the plot as a pair of frequency/magnitude vectors */
    std::pair<const std::vector<float>&, const std::vector<float>&> plotFilterMagnitudeResponse();

    /** Updates the internal juce::Path with a new frequency response */
    void updateFilterPlot();

    /** Returns the current path. */
    [[nodiscard]] const auto& getPath() const { return plotPath; }

    /** Users should implement this function to perform the filtering process. */
    std::function<void (const float*, float*, int)> runFilterCallback;

    const Params params;

    juce::CriticalSection pathMutex {};

private:
    void computeFrequencyResponse();

    static std::vector<float> generateLogSweep (int nSamples, float sampleRate, float startFreqHz, float endFreqHz);
    static std::vector<float> fftFreqs (int N, float T);
    static void freqSmooth (const std::vector<float>& magsDB, std::vector<float>& magsSmoothDB, float smFactor = 1.0f / 24.0f);

    const SpectrumPlotBase& base;
    juce::dsp::FFT fft;
    const int fftSize;

    juce::Path plotPath;

    const std::vector<float> sweepBuffer, freqAxis;
    std::vector<float> filterBuffer;
    std::vector<float> sweepFFT, filtFFT;
    std::vector<float> magResponseDB, magResponseSmoothDB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericFilterPlotter)
};
} // namespace chowdsp
