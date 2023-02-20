#include "chowdsp_PresetsFileInterface.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_utils
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#endif

namespace chowdsp::presets::frontend
{
FileInterface::FileInterface (PresetManager& manager,
                              SettingsInterface* settingsFace)
    : presetManager (manager), settingsInterface (settingsFace)
{
}

void FileInterface::savePreset()
{
    jassert (savePresetCallback != nullptr);
    savePresetCallback (presetManager.savePresetState());
}

void FileInterface::resaveCurrentPreset()
{
    const auto* currentPreset = presetManager.getCurrentPreset();
    jassert (currentPreset != nullptr && currentPreset->getPresetFile().existsAsFile() && ! (currentPreset->isFactoryPreset));

    presetManager.saveUserPreset (
        currentPreset->getPresetFile(),
        Preset {
            currentPreset->getName(),
            currentPreset->getVendor(),
            presetManager.savePresetState(),
            currentPreset->getCategory(),
            currentPreset->getPresetFile() });
}

void FileInterface::deleteCurrentPreset()
{
    const auto* currentPreset = presetManager.getCurrentPreset();
    jassert (currentPreset != nullptr && currentPreset->getPresetFile().existsAsFile() && ! (currentPreset->isFactoryPreset));

    if (checkDeletePresetCallback != nullptr && ! checkDeletePresetCallback (*currentPreset))
        return;

    currentPreset->getPresetFile().deleteFile();
    if (presetManager.getDefaultPreset() != nullptr)
        presetManager.loadPreset (*presetManager.getDefaultPreset());
    presetManager.loadUserPresetsFromFolder (presetManager.getUserPresetPath());
}

void FileInterface::loadPresetFromFile()
{
    constexpr auto fileBrowserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser = std::make_shared<juce::FileChooser> ("Load Preset",
                                                       presetManager.getUserPresetPath(),
                                                       "*" + presetManager.getPresetFileExtension());
    fileChooser->launchAsync (fileBrowserFlags,
                              [this] (const juce::FileChooser& fc)
                              {
                                  if (fc.getResults().isEmpty())
                                      return;

                                  Preset newPreset { fc.getResult() };
                                  if (! newPreset.isValid())
                                  {
                                      failedToLoadPresetCallback (newPreset);
                                      return;
                                  }

                                  presetManager.loadPreset (newPreset);
                              });
}

void FileInterface::goToUserPresetsFolder() const
{
    if (presetManager.getUserPresetPath().isDirectory())
        presetManager.getUserPresetPath().startAsProcess();
}

void FileInterface::chooseUserPresetsFolder()
{
    jassert (hasSettingsInterface()); // this implementation requires a settings interface!
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_utils
    constexpr auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
    fileChooser = std::make_shared<juce::FileChooser> ("Choose User Preset Folder");
    {
        fileChooser->launchAsync (
            folderChooserFlags,
            [this] (const juce::FileChooser& chooser)
            {
                if (chooser.getResults().isEmpty())
                    return;

                settingsInterface->setUserPresetsPath (chooser.getResult());
            });
    }
#endif
}

juce::File FileInterface::getFileForPreset (const Preset& preset) const
{
    jassert (presetManager.getUserPresetPath().isDirectory());
    return presetManager.getUserPresetPath().getChildFile (preset.getName() + presetManager.getPresetFileExtension());
}
} // namespace chowdsp::presets::frontend
