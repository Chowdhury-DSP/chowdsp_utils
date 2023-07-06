#include "chowdsp_PresetManager.h"

namespace
{
const juce::String defaultUserPresetsName { "User" };

template <typename PresetMapType, typename OpType>
void doForAllPresetsForUser (int userStartPresetID, PresetMapType& presetMap, OpType&& op)
{
    int presetID = userStartPresetID;
    for (auto presetMapIter = presetMap.find (presetID); presetMapIter != presetMap.end(); presetMapIter = presetMap.find (++presetID))
        op (presetMapIter);
}
} // namespace

namespace chowdsp
{
PresetManager::PresetManager (juce::AudioProcessorValueTreeState& vtState) : vts (vtState),
                                                                             processor (vts.processor),
                                                                             userPresetsName (defaultUserPresetsName)
{
    for (auto* param : processor.getParameters())
    {
        if (const auto* paramCast = dynamic_cast<juce::RangedAudioParameter*> (param))
            vts.addParameterListener (paramCast->paramID, this);
    }

    userIDMap.insert ({ userPresetsName, userUserIDStart });
}

PresetManager::~PresetManager()
{
    for (auto* param : processor.getParameters())
    {
        if (const auto* paramCast = dynamic_cast<juce::RangedAudioParameter*> (param))
            vts.removeParameterListener (paramCast->paramID, this);
    }
}

void PresetManager::parameterChanged (const juce::String&, float)
{
    if (! isDirty)
        setIsDirty (true);
}

void PresetManager::loadPresetFromIndex (int index)
{
    if (getCurrentPreset() != nullptr && index == getCurrentPresetIndex())
        return;

    const Preset* presetToLoad = getPresetForIndex (index);
    if (presetToLoad != nullptr)
        loadPreset (*presetToLoad);
}

std::pair<const int, Preset>& PresetManager::addFactoryPreset (Preset&& preset)
{
    const auto& vendor = preset.getVendor();
    int presetID = 0;
    if (const auto userIDMapIter = userIDMap.find (vendor); userIDMapIter != userIDMap.end())
        presetID = userIDMapIter->second;

    while (presetMap.count (presetID) > 0)
        presetID++;

    auto presetResultPair = presetMap.insert ({ presetID, std::move (preset) });
    return *presetResultPair.first;
}

void PresetManager::addPresets (std::vector<Preset>& presets)
{
    for (auto& preset : presets)
    {
        if (preset.isValid())
            addFactoryPreset (std::move (preset));
    }

    listeners.call (&Listener::presetListUpdated);
}

const Preset* PresetManager::findPreset (const Preset& presetToFind) const
{
    for (auto& [_, preset] : presetMap)
    {
        if (preset == presetToFind)
            return &preset;
    }

    return nullptr;
}

void PresetManager::setDefaultPreset (Preset&& newDefaultPreset)
{
    // default preset must be a valid preset!
    jassert (newDefaultPreset.isValid());

    auto* foundDefaultPreset = findPreset (newDefaultPreset);

    if (foundDefaultPreset != nullptr)
    {
        defaultPreset = foundDefaultPreset;
        return;
    }

    // default preset is not in factory presets (yet)
    auto& addedPresetPair = addFactoryPreset (std::move (newDefaultPreset));
    defaultPreset = &addedPresetPair.second;
}

void PresetManager::loadDefaultPreset()
{
    // tried to load a default preset before assigning one!
    jassert (defaultPreset != nullptr);

    if (defaultPreset != nullptr)
        loadPreset (*defaultPreset);
}

void PresetManager::saveUserPreset (const juce::File& file)
{
    auto stateXml = savePresetState();
    const auto name = file.getFileNameWithoutExtension();

    keepAlivePreset = std::make_unique<Preset> (name, userPresetsName, *stateXml);
    if (keepAlivePreset != nullptr)
    {
        keepAlivePreset->toFile (file);
        loadPreset (*keepAlivePreset);

        loadUserPresetsFromFolder (getUserPresetPath());
    }
}

void PresetManager::setIsDirty (bool shouldBeDirty)
{
    isDirty = shouldBeDirty;
    listeners.call (&Listener::presetDirtyStatusChanged);
}

void PresetManager::loadPreset (const Preset& preset)
{
    // we need to set current preset before loading its state,
    // since `loadPresetState()` might need to know the name
    // of the current preset that it's loading, or something like that.
    currentPreset = &preset;
    loadPresetState (preset.getState());

    setIsDirty (false);
    listeners.call (&Listener::selectedPresetChanged);

    // For some reason, when VST3 in Ableton Live (Win64), calling updateHostDisplay
    // causes the plugin to load the preset at index 1. @TODO: figure out the problem!
    if (processor.wrapperType == juce::AudioProcessor::WrapperType::wrapperType_VST3)
        return;

#if JUCE_VERSION > 0x60007
    processor.updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withProgramChanged (true));
#else
    processor.updateHostDisplay();
#endif
}

std::unique_ptr<juce::XmlElement> PresetManager::savePresetState()
{
    auto xml = vts.state.createXml();
    xml->deleteAllChildElementsWithTagName (presetStateTag);

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move")
    return std::move (xml);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

void PresetManager::loadPresetState (const juce::XmlElement* xml)
{
    if (auto* curPreset = getCurrentPreset())
    {
        const auto newPresetName = curPreset->getName();
        DBG ("Loading preset: " + newPresetName);
    }

    auto newState = juce::ValueTree::fromXml (*xml);
    vts.replaceState (newState);
}

Preset PresetManager::loadUserPresetFromFile (const juce::File& file)
{
    return Preset { file };
}

int PresetManager::getIndexForPreset (const Preset& preset) const
{
    int counter = 0;
    for (const auto& [_, presetToCheck] : presetMap)
    {
        if (preset == presetToCheck)
            return counter;

        counter++;
    }

    // preset not found!!
    jassertfalse;
    return 0;
}

const Preset* PresetManager::getPresetForIndex (int index) const
{
    int counter = 0;
    for (auto& [_, preset] : presetMap)
    {
        if (counter++ == index)
            return &preset;
    }

    return nullptr;
}

void PresetManager::setUserPresetConfigFile (const juce::String& presetConfigFilePath)
{
    userPresetConfigPath = presetConfigFilePath;

    auto userPresetPath = getUserPresetPath();
    if (userPresetPath.isDirectory())
        loadUserPresetsFromFolder (userPresetPath);
}

juce::File PresetManager::getUserPresetConfigFile() const
{
    return getUserPresetConfigFile (userPresetConfigPath);
}

juce::File PresetManager::getUserPresetConfigFile (const juce::String& presetConfigFilePath)
{
    juce::File updatePresetFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
    return updatePresetFile.getChildFile (presetConfigFilePath);
}

void PresetManager::setUserPresetPath (const juce::File& file)
{
    if (file == juce::File())
        return;

    auto config = getUserPresetConfigFile();
    config.deleteFile();
    config.create();
    config.replaceWithText (file.getFullPathName());
    loadUserPresetsFromFolder (file);
}

juce::File PresetManager::getUserPresetPath() const
{
    return getUserPresetPath (userPresetConfigPath);
}

juce::File PresetManager::getUserPresetPath (const juce::String& presetConfigFilePath)
{
    auto userPresetConfigFile = getUserPresetConfigFile (presetConfigFilePath);
    if (userPresetConfigFile.existsAsFile())
        return userPresetConfigFile.loadFileAsString();

    return {};
}

std::vector<const Preset*> PresetManager::getUserPresets() const
{
    std::vector<const Preset*> userPresets;

    doForAllPresetsForUser (userIDMap.at (userPresetsName), presetMap, [&userPresets] (auto& presetMapIter)
                            { userPresets.push_back (&presetMapIter->second); });

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move")
    return std::move (userPresets);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

void PresetManager::setUserPresetName (const juce::String& newName)
{
    if (newName == userPresetsName)
        return;

    auto actualNewName = newName.isEmpty() ? defaultUserPresetsName : newName;

    if (const auto userIDMapIter = userIDMap.find (userPresetsName); userIDMapIter != userIDMap.end()) // previous user name was the default!
    {
        // move existing user presets to new username
        doForAllPresetsForUser (
            userIDMapIter->second,
            presetMap,
            [&newName] (auto presetMapIter)
            {
                auto& preset = presetMapIter->second;
                preset.setVendor (newName);

                jassert (preset.getPresetFile() != juce::File());
                preset.toFile (preset.getPresetFile());
            });
    }

    userIDMap.erase (userPresetsName);
    userPresetsName = actualNewName;
    userIDMap.insert_or_assign (userPresetsName, userUserIDStart);

    loadUserPresetsFromFolder (getUserPresetPath());
}

void PresetManager::loadUserPresetsFromFolder (const juce::File& file)
{
    std::vector<Preset> presets;
    for (const auto& f : file.findChildFiles (juce::File::findFiles, true))
        presets.push_back (loadUserPresetFromFile (f));

    // delete old user presets
    doForAllPresetsForUser (userIDMap[userPresetsName], presetMap, [this] (auto& presetMapIter)
                            { presetMap.erase (presetMapIter); });

    addPresets (presets);
}

std::unique_ptr<juce::XmlElement> PresetManager::saveXmlState() const
{
    if (currentPreset == nullptr)
        return {};

    auto presetXml = std::make_unique<juce::XmlElement> (presetStateTag);

    presetXml->setAttribute (presetDirtyTag, (int) isDirty);
    presetXml->addChildElement (currentPreset->toXml().release());

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move")
    return std::move (presetXml);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

void PresetManager::loadXmlState (const juce::XmlElement* xml)
{
    if (xml == nullptr)
    {
        loadDefaultPreset();
        return;
    }

    if (xml->getTagName() != presetStateTag.toString())
    {
        loadDefaultPreset();
        return;
    }

    keepAlivePreset = std::make_unique<Preset> (xml->getChildByName (Preset::presetTag));
    if (keepAlivePreset != nullptr)
        loadPreset (*keepAlivePreset);

    setIsDirty ((bool) xml->getIntAttribute (presetDirtyTag));
}

const juce::Identifier PresetManager::presetStateTag { "Preset_State" };
const juce::Identifier PresetManager::presetDirtyTag { "preset_dirty" };

} // namespace chowdsp
