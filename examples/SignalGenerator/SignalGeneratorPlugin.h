#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include <chowdsp_sources/chowdsp_sources.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

struct SignalGenParams : chowdsp::ParamHolder
{
    SignalGenParams()
    {
        add (freqParam,
             gainParam,
             typeParam,
             upsampleParam,
             waveshapeParam);
    }

    chowdsp::FreqHzParameter::Ptr freqParam {
        juce::ParameterID { "freq", 100 },
        "Frequency",
        chowdsp::ParamUtils::createNormalisableRange (50.0f, 50000.0f, 2500.0f),
        1000.0f
    };

    chowdsp::GainDBParameter::Ptr gainParam {
        juce::ParameterID { "gain", 100 },
        "Gain",
        juce::NormalisableRange { -45.0f, 12.0f },
        -24.0f
    };

    chowdsp::ChoiceParameter::Ptr typeParam {
        juce::ParameterID { "type", 100 },
        "Tone Type",
        juce::StringArray { "Sine",
                            "Saw",
                            "Square",
                            "Triangle",
                            "Additive Saw" },
        0
    };

    chowdsp::ChoiceParameter::Ptr upsampleParam {
        juce::ParameterID { "upsample", 100 },
        "Upsample",
        juce::StringArray { "1x", "2x", "3x", "4x" },
        0
    };

    chowdsp::ChoiceParameter::Ptr waveshapeParam {
        juce::ParameterID { "waveshape", 100 },
        "Waveshaper",
        juce::StringArray { "None",
                            "Hard Clip",
                            "Tanh Clip",
                            "Cubic Clip",
                            "9th-Order Clip",
                            "Full Wave Rectify",
                            "West Coast",
                            "Wave Multiply",
                            "Sine Clip" },
        0
    };
};

class SignalGeneratorPlugin : public chowdsp::PluginBase<chowdsp::PluginStateImpl<SignalGenParams>>
{
public:
    SignalGeneratorPlugin();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    void prepareTones (double sampleRate, int blockSize);
    void setUpSampleChoice();

    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> freqHzParamSmoothed;

    chowdsp::SineWave<float> sine;
    chowdsp::SawtoothWave<float> saw;
    chowdsp::SquareWave<float> square;
    chowdsp::TriangleWave<float> triangle;
    static constexpr size_t numHarmonics = 400;
    chowdsp::AdditiveOscillator<numHarmonics, chowdsp::AdditiveOscSineApprox::AbsApprox> additiveSaw;
    chowdsp::Gain<float> gain;

    using AAFilter = chowdsp::ButterworthFilter<12>;
    juce::AudioBuffer<float> upsampledBuffer;
    chowdsp::Downsampler<float, AAFilter, false> resample2;
    chowdsp::Downsampler<float, AAFilter, false> resample3;
    chowdsp::Downsampler<float, AAFilter, false> resample4;

    chowdsp::Downsampler<float, AAFilter, false>* resampler = nullptr;
    int previousUpSampleChoice = 0;

    chowdsp::SharedLookupTableCache lookupTableCache;
    chowdsp::ADAAHardClipper<float> adaaHardClipper { &lookupTableCache.get() };
    chowdsp::ADAATanhClipper<float> adaaTanhClipper { &lookupTableCache.get() };
    chowdsp::ADAASoftClipper<float> adaaCubicClipper { &lookupTableCache.get() };
    chowdsp::ADAASoftClipper<float, 9> adaa9thOrderClipper { &lookupTableCache.get() };
    chowdsp::ADAAFullWaveRectifier<float> fullWaveRectifier { &lookupTableCache.get() };
    chowdsp::WestCoastWavefolder<float> westCoastFolder { &lookupTableCache.get() };
    chowdsp::WaveMultiplier<float, 6> waveMultiplyFolder { &lookupTableCache.get() };
    chowdsp::ADAASineClipper<float> adaaSineClipper { &lookupTableCache.get() };

    chowdsp::OvershootLimiter<float> clipGuard { 64 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalGeneratorPlugin)
};
