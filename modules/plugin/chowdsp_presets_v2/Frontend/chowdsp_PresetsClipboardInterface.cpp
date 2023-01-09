#include "chowdsp_PresetsClipboardInterface.h"

namespace chowdsp::PresetsFrontend
{
ClipboardInterface::ClipboardInterface (PresetManager& manager) : presetManager (manager)
{
}

void ClipboardInterface::copyCurrentPreset() const
{
    jassert (presetManager.currentPreset != nullptr && presetManager.currentPreset->isValid());
    juce::SystemClipboard::copyTextToClipboard (presetManager.currentPreset->toJson().dump());
}

bool ClipboardInterface::tryToPastePreset()
{
    try
    {
        Preset newPreset { nlohmann::json::parse (juce::SystemClipboard::getTextFromClipboard().toStdString()) };
        if (! newPreset.isValid())
            return false;

        presetManager.loadPreset (newPreset);
        return true;
    }
    catch (std::exception& e)
    {
        juce::Logger::writeToLog (juce::String { "Unable to load pasted preset! " } + e.what());
        jassertfalse;
        return false;
    }
}

} // namespace chowdsp::PresetsFrontend
