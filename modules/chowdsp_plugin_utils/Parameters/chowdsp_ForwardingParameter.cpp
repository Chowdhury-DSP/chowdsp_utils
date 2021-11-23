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

void ForwardingParameter::setParam (juce::RangedAudioParameter* paramToUse)
{
    internalParam = paramToUse;

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
        internalParam->setValue (newValue);
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
