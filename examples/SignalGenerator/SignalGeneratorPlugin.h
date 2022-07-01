#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

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

    chowdsp::FloatParameter* freqHzParam = nullptr;
    chowdsp::ChoiceParameter* toneTypeParam = nullptr;
    chowdsp::ChoiceParameter* upSampleParam = nullptr;
    chowdsp::FloatParameter* gainDBParam = nullptr;
    chowdsp::ChoiceParameter* waveshaperParam = nullptr;

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

    chowdsp::SharedLookupTableCache lookupTableCache;
    chowdsp::ADAAHardClipper<float> adaaHardClipper { &lookupTableCache.get() };
    chowdsp::ADAATanhClipper<float> adaaTanhClipper { &lookupTableCache.get() };
    chowdsp::ADAASoftClipper<float> adaaCubicClipper { &lookupTableCache.get() };
    chowdsp::ADAASoftClipper<float, 9> adaa9thOrderClipper { &lookupTableCache.get() };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalGeneratorPlugin)
};
