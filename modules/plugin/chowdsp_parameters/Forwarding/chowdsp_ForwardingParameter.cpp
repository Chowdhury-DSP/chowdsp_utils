#include "chowdsp_ForwardingParameter.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>
#endif

namespace
{
const juce::NormalisableRange defaultRange { 0.0f, 1.0f, 0.01f, 1.0f };
} // namespace

namespace chowdsp
{
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
struct ForwardingParameter::ForwardingAttachment
    : private juce::AudioProcessorParameter::Listener
{
    ForwardingAttachment (juce::RangedAudioParameter& internal,
                          juce::RangedAudioParameter& forwarding,
                          PluginState& state)
        : internalParam (internal),
          forwardingParam (forwarding),
          pluginState (state)
    {
        internalParam.addListener (this);
    }

    ~ForwardingAttachment() override
    {
        internalParam.removeListener (this);
    }

    void setNewValue (float value)
    {
        // This set function will set the internal value atomically.
        // the PluginState will then call the relevant listeners from there.
        internalParam.setValue (value);
    }

private:
    void beginGesture()
    {
        if (pluginState.undoManager != nullptr)
            pluginState.undoManager->beginNewTransaction();

        forwardingParam.beginChangeGesture();
    }

    void endGesture()
    {
        forwardingParam.endChangeGesture();
    }

    void parameterValueChanged (int, float value) override
    {
        // Internal parameters should only ever be set from the message thread!
        // We can enforce this, because we have full control over the internal param.
        jassert (juce::MessageManager::existsAndIsCurrentThread());
        forwardingParam.sendValueChangedMessageToListeners (value);
    }

    void parameterGestureChanged (int, bool gestureIsStarting) override
    {
        jassert (juce::MessageManager::existsAndIsCurrentThread());
        if (gestureIsStarting)
            beginGesture();
        else
            endGesture();
    }

    juce::RangedAudioParameter& internalParam;
    juce::RangedAudioParameter& forwardingParam;
    PluginState& pluginState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ForwardingAttachment)
};
#else
// LCOV_EXCL_START
ForwardingParameter::ForwardingAttachment::ForwardingAttachment (juce::RangedAudioParameter& internal, juce::RangedAudioParameter& forwarding, juce::UndoManager* um)
    : internalParam (internal), forwardingParam (forwarding), undoManager (um)
{
    internalParam.addListener (this);
}

ForwardingParameter::ForwardingAttachment::~ForwardingAttachment()
{
    internalParam.removeListener (this);
}

void ForwardingParameter::ForwardingAttachment::beginGesture()
{
    if (undoManager != nullptr)
        undoManager->beginNewTransaction();

    forwardingParam.beginChangeGesture();
}

void ForwardingParameter::ForwardingAttachment::endGesture()
{
    forwardingParam.endChangeGesture();
}

void ForwardingParameter::ForwardingAttachment::setNewValue (float value)
{
    newValue = value;

    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
    {
        cancelPendingUpdate();
        handleAsyncUpdate();
    }
    else
    {
        triggerAsyncUpdate();
    }
}

void ForwardingParameter::ForwardingAttachment::handleAsyncUpdate()
{
    const juce::ScopedValueSetter svs (ignoreCallbacks, true);
    internalParam.setValueNotifyingHost (newValue);
}

void ForwardingParameter::ForwardingAttachment::parameterValueChanged (int, float value)
{
    if (ignoreCallbacks)
        return;

    forwardingParam.sendValueChangedMessageToListeners (value);
}

void ForwardingParameter::ForwardingAttachment::parameterGestureChanged (int, bool gestureIsStarting)
{
    if (gestureIsStarting)
        beginGesture();
    else
        endGesture();
}
// LCOV_EXCL_END
#endif

//=================================================================================
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
ForwardingParameter::ForwardingParameter (const ParameterID& id, PluginState& state, const juce::String& thisDefaultName)
    : juce::RangedAudioParameter (id, thisDefaultName), pluginState (state), defaultName (thisDefaultName)
{
}
#else
// LCOV_EXCL_START
ForwardingParameter::ForwardingParameter (const ParameterID& id, juce::UndoManager* um, const juce::String& thisDefaultName)
    : juce::RangedAudioParameter (id, thisDefaultName), undoManager (um), defaultName (thisDefaultName)
{
}
// LCOV_EXCL_END
#endif

ForwardingParameter::~ForwardingParameter()
{
    // You must call `setParam (nullptr)` before destroying the ForwardingParameter,
    // otherwise it's very difficult to control the order of deletion between the
    // ForwardingParameter and the parameter being forwarded to.
    jassert (internalParam == nullptr);
}

void ForwardingParameter::reportParameterInfoChange (juce::AudioProcessor* processor)
{
#if JUCE_VERSION > 0x60007
    processor->updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withParameterInfoChanged (true));
#else
    processor->updateHostDisplay();
#endif
}

void ForwardingParameter::setParam (juce::RangedAudioParameter* paramToUse, const juce::String& newName, bool deferHostNotification)
{
#if ! JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
    juce::SpinLock::ScopedLockType sl (paramLock); // LCOV_EXCL_LINE
#endif

    if (internalParam != nullptr)
        attachment.reset();

    internalParam = paramToUse;
    internalParamAsModulatable = dynamic_cast<ParamUtils::ModParameterMixin*> (internalParam);
    customName = newName;

    if (processor != nullptr && ! deferHostNotification)
    {
        reportParameterInfoChange (processor);
    }

    if (internalParam != nullptr)
    {
        setValueNotifyingHost (internalParam->getValue());
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
        attachment = std::make_unique<ForwardingAttachment> (*internalParam, *this, pluginState);
#else
        attachment = std::make_unique<ForwardingAttachment> (*internalParam, *this, undoManager); // LCOV_EXCL_LINE
#endif
    }
}

float ForwardingParameter::getValue() const
{
    if (internalParam != nullptr)
        return internalParam->getValue();

    return 0.0f;
}

void ForwardingParameter::setValue (float newValue)
{
#if ! JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
    // LCOV_EXCL_START
    juce::SpinLock::ScopedTryLockType stl (paramLock);
    if (! stl.isLocked())
        return;
        // LCOV_EXCL_END
#endif

    if (internalParam != nullptr && ! juce::approximatelyEqual (internalParam->getValue(), newValue))
        attachment->setNewValue (newValue);
}

float ForwardingParameter::getDefaultValue() const
{
    if (internalParam != nullptr)
        return internalParam->getDefaultValue();

    return 0.0f;
}

juce::String ForwardingParameter::getText (float value, int i) const
{
    if (internalParam != nullptr)
        return internalParam->getText (value, i);

    return {};
}

float ForwardingParameter::getValueForText (const juce::String& text) const
{
    if (internalParam != nullptr)
        return internalParam->getValueForText (text);

    return 0.0f;
}

juce::String ForwardingParameter::getName (int i) const
{
    if (customName.isNotEmpty())
        return customName;

    if (internalParam != nullptr)
        return internalParam->getName (i);

    return defaultName;
}

const juce::NormalisableRange<float>& ForwardingParameter::getNormalisableRange() const
{
    if (internalParam != nullptr)
        return internalParam->getNormalisableRange();

    return defaultRange;
}

bool ForwardingParameter::supportsMonophonicModulation()
{
    if (internalParamAsModulatable == nullptr)
        return false;

    return internalParamAsModulatable->supportsMonophonicModulation();
}

bool ForwardingParameter::supportsPolyphonicModulation()
{
    if (internalParamAsModulatable == nullptr)
        return false;

    return internalParamAsModulatable->supportsPolyphonicModulation();
}

void ForwardingParameter::applyMonophonicModulation (double value)
{
    if (internalParamAsModulatable == nullptr)
    {
        // If this is nullptr, then supportsMonophonicModulation() should have
        // returned false, meaning this method should not have been called!
        jassertfalse;
        return;
    }

    internalParamAsModulatable->applyMonophonicModulation (value);
}

// let's re-enable coverage for these lines when we've got some parameters that implement polyphonic modulation
// LCOV_EXCL_START
void ForwardingParameter::applyPolyphonicModulation (int32_t note_id, int16_t port_index, int16_t channel, int16_t key, double value)
{
    if (internalParamAsModulatable == nullptr)
    {
        // If this is nullptr, then supportsPolyphonicModulation() should have
        // returned false, meaning this method should not have been called!
        jassertfalse;
        return;
    }

    internalParamAsModulatable->applyPolyphonicModulation (note_id, port_index, channel, key, value);
}
// LCOV_EXCL_STOP

void ForwardingParameter::setProcessor (juce::AudioProcessor* processorToUse)
{
    processor = processorToUse;
}
} // namespace chowdsp
