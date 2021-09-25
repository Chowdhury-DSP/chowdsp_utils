namespace chowdsp
{
PresetManager::PresetManager (juce::AudioProcessor* p, juce::AudioProcessorValueTreeState& vtState) : processor (p),
                                                                                                      vts (vtState)
{
    userIDMap.insert ({ "User", userUserIDStart });
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
    iterator->second.toFile (file);
}

void PresetManager::loadPreset (const Preset& preset)
{
    loadPresetState (preset.state.get());
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
