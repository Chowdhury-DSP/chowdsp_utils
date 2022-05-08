#pragma once

#include <chowdsp_presets/chowdsp_presets.h>

/** Classes for adapting the program interface in juce::AudioProcessor */
namespace chowdsp::ProgramAdapter
{
struct BaseProgramAdapter
{
    virtual ~BaseProgramAdapter() = default;

    virtual int getNumPrograms() { return 1; }
    virtual int getCurrentProgram() { return 0; }
    virtual void setCurrentProgram (int) {}

    virtual const juce::String getProgramName (int) { return {}; } // NOLINT(readability-const-return-type): Needs to return a const String for override compatibility
    virtual void changeProgramName (int, const juce::String&) {}
};

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
class PresetsProgramAdapter : public BaseProgramAdapter
{
public:
    PresetsProgramAdapter (std::unique_ptr<PresetManager>& manager) : presetManager (manager) {}

    int getNumPrograms() override
    {
        if (presetManager == nullptr)
            return BaseProgramAdapter::getNumPrograms();

        return presetManager->getNumPresets();
    }

    int getCurrentProgram() override
    {
        if (presetManager == nullptr)
            return 0;

        return presetManager->getCurrentPresetIndex();
    }

    void setCurrentProgram (int index) override
    {
        if (presetManager == nullptr)
            return;

        presetManager->loadPresetFromIndex (index);
    }

    const juce::String getProgramName (int index) override // NOLINT(readability-const-return-type): Needs to return a const String for override compatibility
    {
        if (presetManager == nullptr)
            return BaseProgramAdapter::getProgramName (index);

        return presetManager->getPresetName (index);
    }

private:
    std::unique_ptr<chowdsp::PresetManager>& presetManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsProgramAdapter)
};
#endif // JUCE_MODULE_AVAILABLE_chowdsp_presets
} // namespace chowdsp::ProgramAdapter
