namespace chowdsp
{
PresetManager::PresetManager (juce::AudioProcessorValueTreeState& vtState) : vts (vtState),
                                                                             processor (vts.processor)
{
    for (auto* param : processor.getParameters())
    {
        if (auto* paramCast = dynamic_cast<juce::RangedAudioParameter*> (param))
            vts.addParameterListener (paramCast->paramID, this);
    }

    userIDMap.insert ({ "User", userUserIDStart });
}

PresetManager::~PresetManager()
{
    for (auto* param : processor.getParameters())
    {
        if (auto* paramCast = dynamic_cast<juce::RangedAudioParameter*> (param))
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
    const Preset* presetToLoad = getPresetForIndex (index);

    if (presetToLoad != nullptr)
        loadPreset (*presetToLoad);
}

void PresetManager::addFactoryPreset (Preset&& preset)
{
    const auto& vendor = preset.getVendor();
    int presetID = 0;
    if (userIDMap.find (vendor) != userIDMap.end())
    {
        presetID = userIDMap[vendor];
    }
    else
    {
        while (userIDMap.find (vendor) != userIDMap.end())
            presetID += factoryUserNumIDs;
    }

    while (presetMap.find (presetID) != presetMap.end())
        presetID++;

    presetMap.insert ({ presetID, std::move (preset) });
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

void PresetManager::saveUserPreset (const juce::File& file)
{
    auto stateXml = savePresetState();
    const auto name = file.getFileNameWithoutExtension();

    int presetID = userIDMap["User"];
    while (presetMap.find (presetID) != presetMap.end())
        presetID++;

    auto newPreset = Preset (name, "User", *stateXml.get());
    auto [iterator, success] = presetMap.insert ({ presetID, std::move (newPreset) });

    jassert (success);
    auto& preset = iterator->second;
    preset.toFile (file);
    loadPreset (preset);
}

void PresetManager::setIsDirty (bool shouldBeDirty)
{
    isDirty = shouldBeDirty;
    listeners.call (&Listener::presetDirtyStatusChanged);
}

void PresetManager::loadPreset (const Preset& preset)
{
    currentPreset = &preset;
    loadPresetState (preset.getState());
    setIsDirty (false);
    listeners.call (&Listener::selectedPresetChanged);
}

std::unique_ptr<juce::XmlElement> PresetManager::savePresetState()
{
    return vts.state.createXml();
}

void PresetManager::loadPresetState (const juce::XmlElement* xml)
{
    auto newState = juce::ValueTree::fromXml (*xml);
    vts.replaceState (newState);
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
    juce::File updatePresetFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
    return updatePresetFile.getChildFile (userPresetConfigPath);
}

void PresetManager::setUserPresetPath (const juce::File& file)
{
    auto config = getUserPresetConfigFile();
    config.deleteFile();
    config.create();
    config.replaceWithText (file.getFullPathName());
    loadUserPresetsFromFolder (file);
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
    for (auto& f : file.findChildFiles (juce::File::findFiles, true))
        presets.emplace_back (f);

    // delete old user presets
    int presetID = userIDMap["User"];
    while (presetMap.find (presetID) != presetMap.end())
        presetMap.erase (presetID++);

    addPresets (presets);
}

std::unique_ptr<juce::XmlElement> PresetManager::saveXmlState() const
{
    if (currentPreset == nullptr)
        return {};

    auto presetXml = std::make_unique<juce::XmlElement> (presetStateTag);

    presetXml->setAttribute (presetDirtyTag, (int) isDirty);
    presetXml->addChildElement (currentPreset->toXml().release());

    return std::move (presetXml);
}

void PresetManager::loadXmlState (juce::XmlElement* xml)
{
    if (xml == nullptr)
        return;

    if (xml->getTagName() != presetStateTag.toString())
        return;

    statePreset = std::make_unique<Preset> (xml->getChildByName (Preset::presetTag));
    if (statePreset != nullptr)
        loadPreset (*statePreset);

    setIsDirty ((bool) xml->getIntAttribute (presetDirtyTag));
}

const juce::Identifier PresetManager::presetStateTag { "Preset_State" };
const juce::Identifier PresetManager::presetDirtyTag { "preset_dirty" };

} // namespace chowdsp
