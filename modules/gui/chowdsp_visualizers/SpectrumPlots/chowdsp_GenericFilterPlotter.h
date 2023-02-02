#pragma once

namespace chowdsp
{
class GenericFilterPlotter
{
public:
    struct Params
    {
        float sampleRate = 48000.0f;
        float freqSmoothOctaves = 1.0f / 12.0f;
        int fftOrder = 13;
    };

    explicit GenericFilterPlotter (const SpectrumPlotBase& plotBase, Params&& plotParams);

    std::pair<const std::vector<float>&, const std::vector<float>&> plotFilterMagnitudeResponse();
    void updateFilterPlot();

    const auto& getPath() const { return plotPath; }

    std::function<void(const float*, float*, int)> runFilterCallback;

    const Params params;

private:
    void computeFrequencyResponse();

    static std::vector<float> generateLogSweep(int nSamples, float sampleRate, float startFreqHz, float endFreqHz);
    static std::vector<float> fftFreqs(int N, float T);
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
}
