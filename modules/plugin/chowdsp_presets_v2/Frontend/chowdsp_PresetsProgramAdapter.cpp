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

    return (int) presetManager->getFactoryPresets().size() + 1;
}

int PresetsProgramAdapter::getCurrentProgram()
{
    if (presetManager == nullptr || presetManager->getCurrentPreset() == nullptr)
        return BaseProgramAdapter::getCurrentProgram();

    const auto& factoryPresets = presetManager->getFactoryPresets();
    const auto presetsIter = std::find (factoryPresets.begin(), factoryPresets.end(), *presetManager->getCurrentPreset());
    if (presetsIter != factoryPresets.end())
        return (int) std::distance (factoryPresets.begin(), presetsIter);

    return (int) factoryPresets.size();
}

void PresetsProgramAdapter::setCurrentProgram (int index)
{
    if (presetManager == nullptr)
        return BaseProgramAdapter::setCurrentProgram (index);

    const auto& factoryPresets = presetManager->getFactoryPresets();
    if (index >= (int) factoryPresets.size() || index < 0)
        return BaseProgramAdapter::setCurrentProgram (index);

    presetManager->loadPreset (factoryPresets[(size_t) index]);
}

const juce::String PresetsProgramAdapter::getProgramName (int index) // NOSONAR NOLINT(readability-const-return-type)
{
    if (presetManager == nullptr)
        return BaseProgramAdapter::getProgramName (index);

    const auto& factoryPresets = presetManager->getFactoryPresets();
    if (index > (int) factoryPresets.size() || index < 0)
        return BaseProgramAdapter::getProgramName (index);

    if (index == (int) factoryPresets.size())
        return "User Preset";

    return factoryPresets[(size_t) index].getName();
}
} // namespace chowdsp::presets::frontend
