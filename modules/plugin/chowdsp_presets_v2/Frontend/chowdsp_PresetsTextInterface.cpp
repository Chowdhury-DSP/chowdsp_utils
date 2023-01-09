#include "chowdsp_PresetsTextInterface.h"

namespace chowdsp::PresetsFrontend
{
TextInterface::TextInterface (PresetManager& manager) : presetManager (manager)
{
    listeners += {
        presetManager.currentPreset.changeBroadcaster.connect ([this]
                                                               { updateText(); }),
        presetManager.isPresetDirty.changeBroadcaster.connect ([this]
                                                               { updateText(); }),
    };

    updateText();
}

void TextInterface::updateText()
{
    presetText = {};
    if (auto* currentPreset = presetManager.currentPreset.get())
    {
        if (currentPreset->isValid())
        {
            presetText = currentPreset->getName();
            if (presetManager.isPresetDirty)
                presetText += "*";
        }
    }

    presetTextChangedBroadcaster (presetText);
}
} // namespace chowdsp::PresetsFrontend
