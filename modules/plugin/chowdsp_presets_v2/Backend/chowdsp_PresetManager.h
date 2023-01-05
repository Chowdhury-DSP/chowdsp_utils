#pragma once

namespace chowdsp
{
class PresetManager
{
public:
    template <typename PluginStateType>
    explicit PresetManager (PluginStateType& state)
    {
        state.nonParams.addStateValues ({ &isDirty });
    }

private:
    StateValue<bool> isDirty { "is_preset_dirty", false };

    PresetTree presetTree;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
}
