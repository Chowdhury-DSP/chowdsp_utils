#pragma once

namespace chowdsp::PresetsFrontend
{
NextPrevious::NextPrevious (PresetManager& manager) : presetManager (manager)
{
    presetChangedCallback = presetManager.currentPreset.changeBroadcaster.connect (
        [this]
        {
            if (presetManager.currentPreset == nullptr)
            {
                currentPresetIndex = -1;
                return;
            }

            currentPresetIndex = presetManager.getPresetTree().getIndexForPreset (*presetManager.currentPreset);
        });
}

bool NextPrevious::navigateThroughPresets (bool forward)
{
    auto& presetTree = presetManager.getPresetTree();
    const auto numAvailablePresets = presetTree.getTotalNumberOfPresets();
    if (currentPresetIndex < 0 || numAvailablePresets <= currentPresetIndex)
        return false;

    auto nextPresetIndex = currentPresetIndex + (forward ? 1 : -1);
    if (wrapAtEndOfTree)
        nextPresetIndex = juce::negativeAwareModulo (nextPresetIndex, numAvailablePresets);

    if (auto* nextPreset = presetTree.getPresetByIndex (nextPresetIndex))
    {
        presetManager.loadPreset (*nextPreset);
        return true;
    }

    return false;
}

bool NextPrevious::goToNextPreset()
{
    return navigateThroughPresets (true);
}

bool NextPrevious::goToPreviousPreset()
{
    return navigateThroughPresets (false);
}
} // namespace chowdsp::PresetsFrontend
