#include "chowdsp_ForwardingParameter.h"

namespace
{
const juce::NormalisableRange<float> defaultRange { 0.0f, 1.0f, 0.01f, 1.0f };
} // namespace

namespace chowdsp
{
ForwardingParameter::ForwardingParameter (const juce::String& id, const juce::String& thisDefaultName)
    : juce::RangedAudioParameter (id, thisDefaultName), defaultName (thisDefaultName)
{
}

void ForwardingParameter::setParam (juce::RangedAudioParameter* paramToUse, const juce::String& newName)
{
    if (internalParam != nullptr)
        internalParam->removeListener (this);

    internalParam = paramToUse;

    if (internalParam != nullptr)
        internalParam->addListener (this);

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
        internalParam->setValueNotifyingHost (newValue);
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

void ForwardingParameter::parameterValueChanged (int, float newValue)
{
    if (ignoreCallbacks)
        return;

    sendValueChangedMessageToListeners (newValue);
}

void ForwardingParameter::parameterGestureChanged (int, bool gestureIsStarting)
{
    if (gestureIsStarting)
        beginChangeGesture();
    else
        endChangeGesture();
}
} // namespace chowdsp
