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

void PresetManager::loadPresetFromIdx (int index)
{
    Preset* presetToLoad = nullptr;
    int counter = 0;
    for (auto& [_, preset] : presetMap)
    {
        presetToLoad = &preset;
        if (counter == index)
            break;
    }

    if (presetToLoad != nullptr)
        loadPreset (*presetToLoad);
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
    loadPresetState (preset.state.get());
    setIsDirty (false);
}

std::unique_ptr<juce::XmlElement> PresetManager::savePresetState()
{
    return vts.state.createXml();
}

void PresetManager::loadPresetState (juce::XmlElement* xml)
{
    auto newState = juce::ValueTree::fromXml (*xml);
    vts.replaceState (newState);
}

} // namespace chowdsp
