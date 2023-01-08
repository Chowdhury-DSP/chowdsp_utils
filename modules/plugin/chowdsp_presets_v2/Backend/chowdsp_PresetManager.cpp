#include "chowdsp_PresetManager.h"

namespace chowdsp
{
static bool isPresetAgnosticParameter (const juce::RangedAudioParameter& param, const std::vector<juce::RangedAudioParameter*>& presetAgnosticParams)
{
    //    if (presetAgnosticParams.empty())
    //        return false;

    return std::find_if (
               presetAgnosticParams.begin(),
               presetAgnosticParams.end(),
               [&param] (const juce::RangedAudioParameter* presetAgnosticParam)
               {
                   return param.paramID == presetAgnosticParam->paramID;
               })
           == presetAgnosticParams.end();
}

void PresetManager::initializeParameterListeners (ParamHolder& params, ParameterListeners& listeners)
{
    params.doForAllParameters (
        [this, &listeners] (auto& param, size_t)
        {
            if (isPresetAgnosticParameter (param, presetAgnosticParameters))
                return;

            parameterListeners += {
                listeners.addParameterListener (param, ParameterListenerThread::MessageThread, [this]
                                                { isPresetDirty.set (true); })
            };
        });
}

void PresetManager::loadPreset (const Preset& preset)
{
    // we need to set current preset before loading its state,
    // since `loadPresetState()` might need to know the name
    // of the current preset that it's loading, or something like that.
    currentPreset = &preset;
    loadPresetState (preset.getState()); // @TODO: could this throw?

    pluginState.getParameterListeners().updateBroadcastersFromMessageThread();

    isPresetDirty.set (false);
    currentPresetChangedBroadcaster();

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
}

nlohmann::json PresetManager::savePresetState()
{
    // @TODO: what about parameters that should be preset-agnostic?
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
    if (auto* curPreset = getCurrentPreset())
    {
        const auto newPresetName = curPreset->getName();
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

void PresetManager::addPresets (std::vector<Preset>&& presets)
{
    for (auto& preset : presets)
    {
        if (preset.isValid())
            presetTree.insertPreset (std::move (preset));
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
    presetTree.removePresets ([this] (const Preset& preset)
                              { return preset.getVendor() == userPresetsVendor; });

    addPresets (std::move (presets));
}
} // namespace chowdsp
