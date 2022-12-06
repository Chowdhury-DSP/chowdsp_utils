#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

/** Dummy plugin used for unit tests */
class DummyPlugin : public chowdsp::PluginBase<DummyPlugin>
{
public:
    explicit DummyPlugin (bool withPresetMgr = false) : chowdsp::PluginBase<DummyPlugin> (&undoManager)
    {
        juce::ignoreUnused (withPresetMgr);
#if JUCE_MODULE_AVAILABLE_chowdsp_presets
        if (withPresetMgr)
            presetManager = std::make_unique<chowdsp::PresetManager> (vts);
#endif
    }

    static void addParameters (Parameters& params)
    {
        chowdsp::ParamUtils::emplace_param<juce::AudioParameterFloat> (params, "dummy", "Dummy", 0.0f, 1.0f, 0.5f);
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
        loadMeasurer.reset (sampleRate, samplesPerBlock);
    }

    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override
    {
        juce::AudioProcessLoadMeasurer::ScopedTimer loadTimer { loadMeasurer, buffer.getNumSamples() };
    }

#if ! JUCE_MODULE_AVAILABLE_foleys_gui_magic
    juce::AudioProcessorEditor* createEditor() override
    {
        struct Editor : juce::AudioProcessorEditor
        {
            explicit Editor (DummyPlugin& plugin) : juce::AudioProcessorEditor (plugin)
            {
                setSize (100, 100);
            }

            void paint (juce::Graphics& g) override
            {
                g.fillAll (juce::Colours::black);
            }
        };
        return new Editor { *this };
    }
#endif

    auto& getUndoManager()
    {
        return undoManager;
    }
    auto& getVTS() { return vts; }
    auto& getLoadMeasurer() { return loadMeasurer; }

private:
    juce::UndoManager undoManager;
    juce::AudioProcessLoadMeasurer loadMeasurer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DummyPlugin)
};
