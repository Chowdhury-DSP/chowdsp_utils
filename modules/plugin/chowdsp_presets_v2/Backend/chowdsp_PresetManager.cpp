#include "chowdsp_PresetManager.h"

namespace chowdsp::presets
{
void PresetManager::initializeListeners (ParamHolder& params, ParameterListeners& paramListeners)
{
    params.doForAllParameters (
        [this, &paramListeners] (auto& param, size_t)
        {
            if (isPresetAgnosticParameter (param))
                return;

            listeners += {
                paramListeners.addParameterListener (
                    param,
                    ParameterListenerThread::MessageThread,
                    [this]
                    {
                        if (! areWeInTheMidstOfAPresetChange)
                            isPresetDirty.set (true);
                    })
            };
        });

    listeners +=
        {
            isPresetDirty.changeBroadcaster.connect (
                [this]
                {
                    if (! areWeInTheMidstOfAPresetChange)
                        presetDirtyStatusBroadcaster();
                }),
            currentPreset.changeBroadcaster.connect (
                [this]
                {
                    if (currentPreset == nullptr)
                        return;

                    juce::ScopedValueSetter svs { areWeInTheMidstOfAPresetChange, true };

                    loadPresetState (currentPreset->getState()); // @TODO: could this throw?
                    pluginState.getParameterListeners().updateBroadcastersFromMessageThread();
                    isPresetDirty.set (false);
                    presetChangedBroadcaster();

                    if (processor != nullptr)
                    {
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
    struct ChangePresetAction : juce::UndoableAction
    {
        explicit ChangePresetAction (PresetManager& mgr, const Preset& preset)
            : manager (mgr),
              performPreset (preset),
              undoPreset (manager.currentPreset->getName(),
                          manager.currentPreset->getVendor(),
                          manager.savePresetState(),
                          manager.currentPreset->getCategory(),
                          manager.currentPreset->getPresetFile()),
              previousStateWasDirty (manager.isPresetDirty)
        {
        }

        bool perform() override
        {
            manager.currentPreset = performPreset;
            return true;
        }

        bool undo() override
        {
            manager.currentPreset = undoPreset;
            manager.isPresetDirty.set (previousStateWasDirty);
            return true;
        }

        int getSizeInUnits() override { return (int) sizeof (*this); }

        PresetManager& manager;
        const Preset& performPreset;
        const Preset undoPreset;
        const bool previousStateWasDirty = false;
    };

    pluginState.callOnMainThread (
        [this, &preset]
        {
            if (currentPreset == nullptr || pluginState.undoManager == nullptr)
            {
                currentPreset = preset;
                return;
            }

            pluginState.undoManager->beginNewTransaction ("Loading preset: " + preset.getName());
            pluginState.undoManager->perform (std::make_unique<ChangePresetAction> (*this, preset).release());
        });
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
    for (auto& preset : std::move (presets))
    {
        if (preset.isValid())
        {
            preset.isFactoryPreset = areFactoryPresets;
            presetTree.insertPreset (std::move (preset));
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
    saveUserPreset (file, Preset { name, userPresetsVendor, savePresetState() });
}

void PresetManager::saveUserPreset (const juce::File& file, Preset&& preset) // NOSONAR
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
    newDefaultPreset.isFactoryPreset = true;
    defaultPreset = &presetTree.insertPreset (std::move (newDefaultPreset));
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
    presetTree.removePresets ([] (const Preset& preset)
                              { return ! preset.isFactoryPreset; });

    addPresets (std::move (presets), false);
}
} // namespace chowdsp::presets
