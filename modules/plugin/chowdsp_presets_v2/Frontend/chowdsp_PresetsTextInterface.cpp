#include "chowdsp_PresetsTextInterface.h"

namespace chowdsp::presets::frontend
{
TextInterface::TextInterface (PresetManager& manager) : presetManager (manager)
{
    listeners += {
        presetManager.getSaveLoadHelper().presetChangedBroadcaster.connect ([this]
                                                                            { updateText(); }),
        presetManager.getSaveLoadHelper().presetDirtyStatusBroadcaster.connect ([this]
                                                                                { updateText(); }),
    };

    updateText();
}

void TextInterface::updateText()
{
    presetText = {};
    if (auto* currentPreset = presetManager.getCurrentPreset())
    {
        if (currentPreset->isValid())
        {
            presetText = currentPreset->getName();
            if (presetManager.getIsPresetDirty())
                presetText += "*";
        }
    }

    presetTextChangedBroadcaster (presetText);
}
} // namespace chowdsp::presets::frontend
