#include "chowdsp_PresetsProgramAdapter.h"

namespace chowdsp::presets::frontend
{
PresetsProgramAdapter::PresetsProgramAdapter (std::unique_ptr<PresetManager>& manager)
    : presetManager (manager)
{
}

int PresetsProgramAdapter::getNumPrograms()
{
    if (presetManager == nullptr)
        return BaseProgramAdapter::getNumPrograms();

    return presetManager->getPresetTree().getTotalNumberOfPresets();
}

int PresetsProgramAdapter::getCurrentProgram()
{
    if (presetManager == nullptr || presetManager->getCurrentPreset() == nullptr)
        return BaseProgramAdapter::getCurrentProgram();

    return presetManager->getPresetTree().getIndexForPreset (*presetManager->getCurrentPreset());
}

void PresetsProgramAdapter::setCurrentProgram (int index)
{
    if (presetManager == nullptr)
        return BaseProgramAdapter::setCurrentProgram (index);

    const auto presetForIndex = presetManager->getPresetTree().getPresetByIndex (index);
    if (presetForIndex == nullptr)
        return BaseProgramAdapter::setCurrentProgram (index);

    presetManager->loadPreset (*presetForIndex);
}

const juce::String PresetsProgramAdapter::getProgramName (int index) // NOSONAR NOLINT(readability-const-return-type)
{
    if (presetManager == nullptr)
        return BaseProgramAdapter::getProgramName (index);

    const auto presetForIndex = presetManager->getPresetTree().getPresetByIndex (index);
    if (presetForIndex == nullptr)
        return BaseProgramAdapter::getProgramName (index);

    return presetForIndex->getName();
}
} // namespace chowdsp::presets::frontend
