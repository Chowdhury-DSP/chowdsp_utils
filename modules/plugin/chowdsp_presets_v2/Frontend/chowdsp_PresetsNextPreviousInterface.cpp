#pragma once

namespace chowdsp::PresetsFrontend
{
NextPrevious::NextPrevious (PresetManager& manager) : presetManager (manager)
{
    presetChangedCallback = presetManager.presetChangedBroadcaster
                                .connect ([this]
                                          { updateCurrentPresetIndex(); });
    updateCurrentPresetIndex();
}

void NextPrevious::updateCurrentPresetIndex()
{
    if (presetManager.getCurrentPreset() == nullptr)
    {
        currentPresetIndex = -1;
        return;
    }

    currentPresetIndex = presetManager.getPresetTree().getIndexForPreset (*presetManager.getCurrentPreset());
}

bool NextPrevious::navigateThroughPresets (bool forward)
{
    const auto& presetTree = presetManager.getPresetTree();
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
