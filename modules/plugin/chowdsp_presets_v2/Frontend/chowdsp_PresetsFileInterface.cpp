#include "chowdsp_PresetsFileInterface.h"

namespace chowdsp::PresetsFrontend
{
FileInterface::FileInterface (PresetManager& manager, const juce::String& presetFileExt)
    : presetManager (manager), presetFileExtension (presetFileExt)
{
    jassert (presetFileExtension[0] == '.'); // invalid extension!
}

void FileInterface::savePreset()
{
    jassert (savePresetCallback != nullptr);
    savePresetCallback (presetManager.savePresetState());
}

void FileInterface::resaveCurrentPreset()
{
    const auto* currentPreset = presetManager.currentPreset.get();
    jassert (currentPreset != nullptr && currentPreset->getPresetFile().existsAsFile() && ! presetManager.isFactoryPreset (*currentPreset));

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
    const auto* currentPreset = presetManager.currentPreset.get();
    jassert (currentPreset != nullptr && currentPreset->getPresetFile().existsAsFile() && ! presetManager.isFactoryPreset (*currentPreset));

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
                                                       "*" + presetFileExtension);
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

void FileInterface::goToUserPresetsFolder()
{
    if (presetManager.getUserPresetPath().isDirectory())
        presetManager.getUserPresetPath().startAsProcess();
}

void FileInterface::chooseUserPresetsFolder()
{
    constexpr auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
    fileChooser = std::make_shared<juce::FileChooser> ("Choose User Preset Folder");
    {
        fileChooser->launchAsync (
            folderChooserFlags,
            [this] (const juce::FileChooser& chooser)
            {
                if (chooser.getResults().isEmpty())
                    return;

                presetManager.setUserPresetPath (chooser.getResult());
            });
    }
}

juce::File FileInterface::getFileForPreset (const Preset& preset) const
{
    jassert (presetManager.getUserPresetPath().isDirectory());
    return presetManager.getUserPresetPath().getChildFile (preset.getName() + presetFileExtension);
}
} // namespace chowdsp::PresetsFrontend
