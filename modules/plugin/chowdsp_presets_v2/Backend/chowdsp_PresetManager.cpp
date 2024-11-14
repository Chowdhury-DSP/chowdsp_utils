#include "chowdsp_PresetManager.h"

#if HAS_CLAP_JUCE_EXTENSIONS
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100)
#include <clap/helpers/preset-discovery-provider.hh>
JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif

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

    if (const auto justSavedPreset = presetTree.findElement (preset))
        loadPreset (*justSavedPreset);
    else
        jassertfalse; // preset was not saved correctly!
}

void PresetManager::setDefaultPreset (Preset&& newDefaultPreset)
{
    // default preset must be a valid preset!
    jassert (newDefaultPreset.isValid());

    if (const auto foundDefaultPreset = presetTree.findElement (newDefaultPreset))
    {
        defaultPreset = &foundDefaultPreset.value();
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

void PresetManager::loadPreset (const Preset& preset)
{
#if HAS_CLAP_JUCE_EXTENSIONS
    const auto presetFile = preset.getPresetFile();
    const auto presetName = preset.getName();
#endif

    saverLoader.loadPreset (preset);

#if HAS_CLAP_JUCE_EXTENSIONS
    if (presetFile == juce::File {})
    {
        clapPresetLoadedBroadcaster (CLAP_PRESET_DISCOVERY_LOCATION_PLUGIN,
                                     nullptr,
                                     presetName.toRawUTF8());
    }
    else
    {
        clapPresetLoadedBroadcaster (CLAP_PRESET_DISCOVERY_LOCATION_FILE,
                                     presetFile.getFullPathName().toRawUTF8(),
                                     nullptr);
    }
#endif
}

#if HAS_CLAP_JUCE_EXTENSIONS
bool PresetManager::loadCLAPPreset (uint32_t location_kind, const char* location, const char* load_key) noexcept
{
    if (location_kind == CLAP_PRESET_DISCOVERY_LOCATION_FILE)
    {
        const auto presetFile = juce::File { location };
        if (! presetFile.existsAsFile())
            return false;

        const auto preset = Preset { presetFile };
        if (! preset.isValid())
            return false;

        loadPreset (preset);
        return true;
    }

    if (location_kind == CLAP_PRESET_DISCOVERY_LOCATION_PLUGIN)
    {
        for (const auto& preset : getFactoryPresets())
        {
            if (preset.getName() == load_key)
            {
                loadPreset (preset);
                return true;
            }
        }
    }

    return false;
}
#endif
} // namespace chowdsp::presets
