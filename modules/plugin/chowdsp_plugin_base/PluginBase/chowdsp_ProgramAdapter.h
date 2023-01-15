#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
#include <chowdsp_presets/chowdsp_presets.h>
#endif

namespace chowdsp
{
/** Classes for adapting the program interface in juce::AudioProcessor */
namespace ProgramAdapter
{
    /** Base class for interfacing with the juce::AudioProcessor program API */
    struct BaseProgramAdapter
    {
        virtual ~BaseProgramAdapter() = default;

        virtual int getNumPrograms() { return 1; }
        virtual int getCurrentProgram() { return 0; }
        virtual void setCurrentProgram (int) {}

        virtual const juce::String getProgramName (int) { return {}; } // NOSONAR NOLINT(readability-const-return-type): Needs to return a const String for override compatibility
        virtual void changeProgramName (int, const juce::String&) {}
    };

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
    /** Interface between chowdsp::PresetManager and the juce::AudioProcessor program API */
    class PresetsProgramAdapter : public BaseProgramAdapter
    {
    public:
        explicit PresetsProgramAdapter (std::unique_ptr<PresetManager>& manager) : presetManager (manager) {}

        int getNumPrograms() override
        {
            if (presetManager == nullptr)
                return BaseProgramAdapter::getNumPrograms();

            return presetManager->getNumPresets();
        }

        int getCurrentProgram() override
        {
            if (presetManager == nullptr)
                return BaseProgramAdapter::getCurrentProgram();

            return presetManager->getCurrentPresetIndex();
        }

        void setCurrentProgram (int index) override
        {
            if (presetManager == nullptr)
                return BaseProgramAdapter::setCurrentProgram (index);

            presetManager->loadPresetFromIndex (index);
        }

        const juce::String getProgramName (int index) override // NOLINT(readability-const-return-type): Needs to return a const String for override compatibility
        {
            if (presetManager == nullptr)
                return BaseProgramAdapter::getProgramName (index);

            return presetManager->getPresetName (index);
        }

    private:
        std::unique_ptr<PresetManager>& presetManager;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsProgramAdapter)
    };
#endif // JUCE_MODULE_AVAILABLE_chowdsp_presets
} // namespace ProgramAdapter
} // namespace chowdsp
