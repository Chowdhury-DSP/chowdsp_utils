#include "chowdsp_ForwardingParameter.h"

namespace
{
const juce::NormalisableRange<float> defaultRange { 0.0f, 1.0f, 0.01f, 1.0f };
} // namespace

namespace chowdsp
{
// @TODO: Add options for UndoManager...
ForwardingParameter::ForwardingAttachment::ForwardingAttachment (juce::RangedAudioParameter& internal, juce::RangedAudioParameter& forwarding)
    : internalParam (internal), forwardingParam (forwarding)
{
    internalParam.addListener (this);
}

ForwardingParameter::ForwardingAttachment::~ForwardingAttachment()
{
    internalParam.removeListener (this);
}

void ForwardingParameter::ForwardingAttachment::beginGesture()
{
    // @TODO: UndoManager call here...
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

//=================================================================================
ForwardingParameter::ForwardingParameter (const juce::String& id, const juce::String& thisDefaultName)
    : juce::RangedAudioParameter (id, thisDefaultName), defaultName (thisDefaultName)
{
}

void ForwardingParameter::setParam (juce::RangedAudioParameter* paramToUse, const juce::String& newName)
{
    if (internalParam != nullptr)
        attachment.reset();

    internalParam = paramToUse;

    if (internalParam != nullptr)
        attachment = std::make_unique<ForwardingAttachment> (*internalParam, *this);

    customName = newName;

    if (processor != nullptr)
#if JUCE_VERSION > 0x60007
        processor->updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withParameterInfoChanged (true));
#else
        processor->updateHostDisplay();
#endif
}

float ForwardingParameter::getValue() const
{
    if (internalParam != nullptr)
        return internalParam->getValue();

    return 0.0f;
}

void ForwardingParameter::setValue (float newValue)
{
    if (internalParam != nullptr)
    {
        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);

        if (newValue != internalParam->getValue())
            attachment->setNewValue (newValue);
    }
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

void ForwardingParameter::setProcessor (juce::AudioProcessor* processorToUse)
{
    processor = processorToUse;
}
} // namespace chowdsp
