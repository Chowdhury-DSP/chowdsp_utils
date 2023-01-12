#include "chowdsp_PresetManager.h"

namespace chowdsp
{
void PresetManager::initializeListeners (ParamHolder& params, ParameterListeners& paramListeners)
{
    params.doForAllParameters (
        [this, &paramListeners] (auto& param, size_t)
        {
            if (isPresetAgnosticParameter (param))
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

bool PresetManager::isPresetAgnosticParameter (const juce::RangedAudioParameter& param) const
{
    return std::find_if (
               presetAgnosticParameters.begin(),
               presetAgnosticParameters.end(),
               [&param] (const juce::RangedAudioParameter* presetAgnosticParam)
               {
                   return param.paramID == presetAgnosticParam->paramID;
               })
           != presetAgnosticParameters.end();
}

nlohmann::json PresetManager::savePresetState()
{
    nlohmann::json state;
    std::as_const (pluginState)
        .getParameters()
        .doForAllParameters (
            [this, &state] (auto& param, size_t)
            {
                if (isPresetAgnosticParameter (param))
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
                if (isPresetAgnosticParameter (param))
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
    saveUserPreset (file, Preset { name, userPresetsVendor, savePresetState() });
}

void PresetManager::saveUserPreset (const juce::File& file, Preset&& preset)
{
    jassert (preset.isValid()); // trying to save an invalid preset??

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

    // default preset is not in factory presets, so let's add it!
    defaultPreset = &presetTree.insertPreset (std::move (newDefaultPreset));
    factoryPresets.emplace_back (defaultPreset);
}

void PresetManager::loadDefaultPreset()
{
    // tried to load a default preset before assigning one!
    jassert (defaultPreset != nullptr);

    if (defaultPreset != nullptr)
        loadPreset (*defaultPreset);
}

bool PresetManager::isFactoryPreset (const Preset& preset) const
{
    return std::find (factoryPresets.begin(), factoryPresets.end(), &preset) == factoryPresets.end();
}

void PresetManager::setUserPresetPath (const juce::File& file)
{
    userPresetPath = file;
    if (file == juce::File())
        return;

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
        if (f.getFileName() == ".DS_Store")
            continue;

        presets.push_back (loadUserPresetFromFile (f));
    }

    // delete old user presets
    presetTree.removePresets ([this] (const Preset& preset) { return isFactoryPreset (preset); });

    addPresets (std::move (presets), false);
}
} // namespace chowdsp
