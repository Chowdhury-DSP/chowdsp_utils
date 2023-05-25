#include "chowdsp_PresetManager.h"

namespace chowdsp::presets
{
PresetManager::PresetManager (PluginState& state,
                              juce::AudioProcessor* plugin,
                              const juce::String& presetFileExtension,
                              std::vector<juce::RangedAudioParameter*>&& presetAgnosticParams,
                              PresetTree::InsertionHelper&& insertionHelper)
    : saverLoader (state, plugin, std::move (presetAgnosticParams)),
      presetTree (&saverLoader.getPresetState(), std::move (insertionHelper)),
      presetFileExt (presetFileExtension)
{
    jassert (presetFileExt[0] == '.'); // invalid file extension!
}

void PresetManager::addPresets (std::vector<Preset>&& presets, bool areFactoryPresets)
{
    if (areFactoryPresets)
    {
        factoryPresets.clear();
        factoryPresets.reserve (presets.size());
    }

    for (auto& preset : std::move (presets))
    {
        if (preset.isValid())
        {
            preset.isFactoryPreset = areFactoryPresets;
            if (areFactoryPresets)
                factoryPresets.emplace_back (preset);
            presetTree.insertElement (std::move (preset));
        }
    }

    presetListUpdatedBroadcaster();
}

Preset PresetManager::getUserPresetForState (const juce::String& presetName, nlohmann::json&& presetState) const
{
    return { presetName, userPresetsVendor, std::move (presetState) };
}

void PresetManager::saveUserPreset (const juce::File& file)
{
    const auto name = file.getFileNameWithoutExtension();
    saveUserPreset (file, Preset { name, userPresetsVendor, saverLoader.savePresetState() });
}

void PresetManager::saveUserPreset (const juce::File& file, Preset&& preset) // NOSONAR
{
    jassert (preset.isValid()); // trying to save an invalid preset??

    if (file.existsAsFile())
        file.deleteFile();
    file.create();
    preset.toFile (file);
    loadUserPresetsFromFolder (getUserPresetPath());

    if (const auto* justSavedPreset = presetTree.findElement (preset))
        loadPreset (*justSavedPreset);
    else
        jassertfalse; // preset was not saved correctly!
}

void PresetManager::setDefaultPreset (Preset&& newDefaultPreset)
{
    // default preset must be a valid preset!
    jassert (newDefaultPreset.isValid());

    if (const auto* foundDefaultPreset = presetTree.findElement (newDefaultPreset))
    {
        defaultPreset = foundDefaultPreset;
        return;
    }

    defaultPreset = &presetTree.insertElement (std::move (newDefaultPreset));
}

void PresetManager::loadDefaultPreset()
{
    // tried to load a default preset before assigning one!
    jassert (defaultPreset != nullptr);

    if (defaultPreset != nullptr)
        loadPreset (*defaultPreset);
}

void PresetManager::setUserPresetPath (const juce::File& file)
{
    if (file == juce::File())
        return;

    userPresetPath = file;
    loadUserPresetsFromFolder (file);
}

juce::File PresetManager::getUserPresetPath() const
{
    return userPresetPath;
}

Preset PresetManager::loadUserPresetFromFile (const juce::File& file)
{
    return Preset { file };
}

void PresetManager::loadUserPresetsFromFolder (const juce::File& file)
{
    jassert (file != juce::File {}); // can't load a non-existent folder!

    std::vector<Preset> presets;
    for (const auto& f : file.findChildFiles (juce::File::findFiles, true))
    {
        if (f.getFileExtension() != presetFileExt)
            continue;

        presets.push_back (loadUserPresetFromFile (f));
    }

    // delete old user presets
    presetTree.removeElements ([] (const Preset& preset)
                               { return ! preset.isFactoryPreset; });

    addPresets (std::move (presets), false);
}
} // namespace chowdsp::presets
