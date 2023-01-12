#include "chowdsp_PresetsTextInterface.h"

namespace chowdsp::PresetsFrontend
{
TextInterface::TextInterface (PresetManager& manager) : presetManager (manager)
{
    listeners += {
        presetManager.presetChangedBroadcaster.connect ([this]
                                                        { updateText(); }),
        presetManager.presetDirtyStatusBroadcaster.connect ([this]
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
} // namespace chowdsp::PresetsFrontend
