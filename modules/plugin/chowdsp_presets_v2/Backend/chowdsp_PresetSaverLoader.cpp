#include "chowdsp_PresetSaverLoader.h"

namespace chowdsp::presets
{
PresetSaverLoader::PresetSaverLoader (PluginState& state,
                                      juce::AudioProcessor* plugin,
                                      std::vector<juce::RangedAudioParameter*>&& presetAgnosticParams)
    : processor (plugin),
      pluginState (state),
      presetAgnosticParameters (std::move (presetAgnosticParams))
{
    state.getNonParameters().addStateValues ({ &currentPreset, &isPresetDirty });
    initializeListeners (state.getParameters(), state.getParameterListeners());
}

void PresetSaverLoader::initializeListeners (ParamHolder& params, ParameterListeners& paramListeners)
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

void PresetSaverLoader::loadPreset (const Preset& preset)
{
    struct ChangePresetAction : juce::UndoableAction
    {
        explicit ChangePresetAction (PresetSaverLoader& sl, Preset&& preset)
            : saverLoader (sl),
              performPreset (std::move (preset)),
              undoPreset (saverLoader.currentPreset->getName(),
                          saverLoader.currentPreset->getVendor(),
                          saverLoader.savePresetState(),
                          saverLoader.currentPreset->getCategory(),
                          saverLoader.currentPreset->getPresetFile()),
              previousStateWasDirty (saverLoader.isPresetDirty)
        {
        }

        bool perform() override
        {
            saverLoader.currentPreset = performPreset;
            return true;
        }

        bool undo() override
        {
            saverLoader.currentPreset = undoPreset;
            saverLoader.isPresetDirty.set (previousStateWasDirty);
            return true;
        }

        int getSizeInUnits() override { return (int) sizeof (*this); }

        PresetSaverLoader& saverLoader;
        const Preset performPreset;
        const Preset undoPreset;
        const bool previousStateWasDirty = false;
    };

    pluginState.callOnMainThread (
        [this, preset = preset]() mutable
        {
            if (currentPreset == nullptr || pluginState.undoManager == nullptr)
            {
                currentPreset = OptionalPointer<const Preset> { std::move (preset) };
                return;
            }

            pluginState.undoManager->beginNewTransaction ("Loading preset: " + preset.getName());
            pluginState.undoManager->perform (std::make_unique<ChangePresetAction> (*this, std::move (preset)).release());
        });
}

bool PresetSaverLoader::isPresetAgnosticParameter (const juce::RangedAudioParameter& param) const
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

nlohmann::json PresetSaverLoader::savePresetParameters()
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

void PresetSaverLoader::loadPresetParameters (const nlohmann::json& state)
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
} // namespace chowdsp::presets
