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

    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>&) override {}

#if ! JUCE_MODULE_AVAILABLE_foleys_gui_magic
    juce::AudioProcessorEditor* createEditor() override
    {
        return nullptr;
    }
#endif

    juce::UndoManager& getUndoManager()
    {
        return undoManager;
    }

    juce::AudioProcessorValueTreeState& getVTS() { return vts; }

private:
    juce::UndoManager undoManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DummyPlugin)
};
