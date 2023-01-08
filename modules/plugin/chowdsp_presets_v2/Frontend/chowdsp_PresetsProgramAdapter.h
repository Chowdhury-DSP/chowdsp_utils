#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_base
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#else
#include "../../chowdsp_plugin_base/PluginBase/chowdsp_ProgramAdapter.h"
#endif

namespace chowdsp
{
/** Interface between chowdsp::PresetManager and the juce::AudioProcessor program API */
class PresetsProgramAdapter : public ProgramAdapter::BaseProgramAdapter
{
public:
    explicit PresetsProgramAdapter (std::unique_ptr<PresetManager>& manager);

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override; // NOLINT(readability-const-return-type): Needs to return a const String for override compatibility

private:
    std::unique_ptr<PresetManager>& presetManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsProgramAdapter)
};
} // namespace chowdsp
