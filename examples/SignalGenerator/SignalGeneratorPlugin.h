#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include <chowdsp_dsp/chowdsp_dsp.h>

class SignalGeneratorPlugin : public chowdsp::PluginBase<SignalGeneratorPlugin>
{
public:
    SignalGeneratorPlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    void prepareTones (double sampleRate, int blockSize);
    void setUpSampleChoice();

    std::atomic<float>* freqHzParam = nullptr;
    std::atomic<float>* toneTypeParam = nullptr;
    std::atomic<float>* upSampleParam = nullptr;
    std::atomic<float>* gainDBParam = nullptr;

    chowdsp::SineWave<float> sine;
    chowdsp::SawtoothWave<float> saw;
    chowdsp::SquareWave<float> square;
    juce::dsp::Gain<float> gain;

    juce::AudioBuffer<float> upsampledBuffer;
    chowdsp::Downsampler<float, 12> resample2;
    chowdsp::Downsampler<float, 12> resample3;
    chowdsp::Downsampler<float, 12> resample4;

    chowdsp::Downsampler<float, 12>* resampler = nullptr;
    int previousUpSampleChoice = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalGeneratorPlugin)
};
