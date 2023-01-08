#include "chowdsp_PresetManager.h"

namespace chowdsp
{
static bool isPresetAgnosticParameter (const juce::RangedAudioParameter& param, const std::vector<juce::RangedAudioParameter*>& presetAgnosticParams)
{
    return std::find_if (
               presetAgnosticParams.begin(),
               presetAgnosticParams.end(),
               [&param] (const juce::RangedAudioParameter* presetAgnosticParam)
               {
                   return param.paramID == presetAgnosticParam->paramID;
               })
           != presetAgnosticParams.end();
}

void PresetManager::initializeListeners (ParamHolder& params, ParameterListeners& paramListeners)
{
    params.doForAllParameters (
        [this, &paramListeners] (auto& param, size_t)
        {
            if (isPresetAgnosticParameter (param, presetAgnosticParameters))
                return;

            listeners += {
                paramListeners.addParameterListener (param, ParameterListenerThread::MessageThread, [this]
                                                     { isPresetDirty.set (true); })
            };
        });

    listeners +=
        {
            currentPreset.changeBroadcaster.connect (
                [this]
                {
                    if (currentPreset == nullptr)
                        return;

                    loadPresetState (currentPreset->getState()); // @TODO: could this throw?
                    pluginState.getParameterListeners().updateBroadcastersFromMessageThread();
                    isPresetDirty.set (false);

                    if (processor != nullptr)
                    {
            // For some reason, when VST3 in Ableton Live (Win64), calling updateHostDisplay
            // causes the plugin to load the preset at index 1. @TODO: figure out the problem!
            //        if (processor->wrapperType == juce::AudioProcessor::WrapperType::wrapperType_VST3)
            //            return;

#if JUCE_VERSION > 0x60007
                        processor->updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withProgramChanged (true));
#else
                        processor->updateHostDisplay();
#endif
                    }
                })
        };
}

void PresetManager::loadPreset (const Preset& preset)
{
    pluginState.callOnMainThread ([this, &preset]
                                  { currentPreset = preset; });
}

nlohmann::json PresetManager::savePresetState()
{
    nlohmann::json state;
    std::as_const (pluginState)
        .getParameters()
        .doForAllParameters (
            [this, &state] (auto& param, size_t)
            {
                if (isPresetAgnosticParameter (param, presetAgnosticParameters))
                    return;

                state[param.paramID.toStdString()] = ParameterTypeHelpers::getValue (param);
            });
    return state;
}

void PresetManager::loadPresetState (const nlohmann::json& state)
{
    if (currentPreset != nullptr)
    {
        const auto newPresetName = currentPreset->getName();
        juce::Logger::writeToLog ("Loading preset: " + newPresetName);
    }

    pluginState
        .getParameters()
        .doForAllParameters (
            [this, &state] (auto& param, size_t)
            {
                if (isPresetAgnosticParameter (param, presetAgnosticParameters))
                    return;

                if (state.contains (param.paramID.toStdString()))
                {
                    ParameterTypeHelpers::setValue (state[param.paramID.toStdString()], param);
                    return;
                }

                ParameterTypeHelpers::resetParameter (param);
            });
}

void PresetManager::addPresets (std::vector<Preset>&& presets, bool areFactoryPresets)
{
    for (auto& preset : presets)
    {
        if (preset.isValid())
        {
            const auto& justAddedPreset = presetTree.insertPreset (std::move (preset));
            if (areFactoryPresets)
                factoryPresets.emplace_back (&justAddedPreset);
        }
    }

    presetListUpdatedBroadcaster();
}

void PresetManager::saveUserPreset (const juce::File& file)
{
    const auto name = file.getFileNameWithoutExtension();

    auto preset = Preset { name, userPresetsVendor, savePresetState() };
    jassert (preset.isValid());

    if (file.existsAsFile())
        file.deleteFile();
    file.create();
    preset.toFile (file);
    loadUserPresetsFromFolder (getUserPresetPath());

    if (auto* justSavedPreset = presetTree.findPreset (preset))
        loadPreset (*justSavedPreset);
    else
        jassertfalse; // preset was not saved correctly!
}

void PresetManager::setDefaultPreset (Preset&& newDefaultPreset)
{
    // default preset must be a valid preset!
    jassert (newDefaultPreset.isValid());

    if (const auto* foundDefaultPreset = presetTree.findPreset (newDefaultPreset))
    {
        defaultPreset = foundDefaultPreset;
        return;
    }

    // default preset is not in factory presets (yet)
    defaultPreset = &presetTree.insertPreset (std::move (newDefaultPreset));
}

void PresetManager::loadDefaultPreset()
{
    // tried to load a default preset before assigning one!
    jassert (defaultPreset != nullptr);

    if (defaultPreset != nullptr)
        loadPreset (*defaultPreset);
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
    auto userPresetConfigFile = getUserPresetConfigFile();
    if (userPresetConfigFile.existsAsFile())
        return userPresetConfigFile.loadFileAsString();

    return {};
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
        presets.push_back (loadUserPresetFromFile (f));

    // delete old user presets
    presetTree.removePresets ([this] (const Preset& preset) -> bool
                              { return std::find (factoryPresets.begin(), factoryPresets.end(), &preset) == factoryPresets.end(); });

    addPresets (std::move (presets), false);
}
} // namespace chowdsp
