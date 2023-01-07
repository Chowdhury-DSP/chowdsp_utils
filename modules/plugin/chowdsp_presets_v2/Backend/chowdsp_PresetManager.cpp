#include "chowdsp_PresetManager.h"

namespace chowdsp
{
void PresetManager::setUserPresetConfigFile (const juce::String& presetConfigFilePath)
{
    userPresetConfigPath = presetConfigFilePath;

    auto userPresetPath = getUserPresetPath();
//    if (userPresetPath.isDirectory())
//        loadUserPresetsFromFolder (userPresetPath);
}

juce::File PresetManager::getUserPresetConfigFile() const
{
    juce::File updatePresetFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
    return updatePresetFile.getChildFile (userPresetConfigPath);
}

void PresetManager::setUserPresetPath (const juce::File& file)
{
    if (file == juce::File())
        return;

    auto config = getUserPresetConfigFile();
    config.deleteFile();
    config.create();
    config.replaceWithText (file.getFullPathName());
//    loadUserPresetsFromFolder (file);
}

juce::File PresetManager::getUserPresetPath() const
{
    auto userPresetConfigFile = getUserPresetConfigFile();
    if (userPresetConfigFile.existsAsFile())
        return userPresetConfigFile.loadFileAsString();

    return {};
}

void PresetManager::loadUserPresetsFromFolder (const juce::File& file)
{
    std::vector<Preset> presets;
//    for (const auto& f : file.findChildFiles (juce::File::findFiles, true))
//        presets.push_back (loadUserPresetFromFile (f));

    // delete old user presets
//    doForAllPresetsForUser (userIDMap[userPresetsName], presetMap, [this] (auto& presetMapIter)
//                            { presetMap.erase (presetMapIter); });
//
//    addPresets (presets);
}
}
