#include "chowdsp_CLAPProcessorExtensions.h"

namespace chowdsp::CLAPExtensions
{
void CLAPProcessorExtensions::initialise (juce::AudioProcessor& baseProcessor)
{
    processor = &baseProcessor;
    jassert (processor != nullptr); // initialized with a NULL processor?

    for (auto* param : processor->getParameters())
    {
        if (auto* modulatableParameter = dynamic_cast<ParamUtils::ModParameterMixin*> (param))
            modulatableParameters[param] = modulatableParameter;
    }
}

bool CLAPProcessorExtensions::supportsDirectEvent (uint16_t space_id, uint16_t type)
{
    if (space_id != CLAP_CORE_EVENT_SPACE_ID)
        return false;

    // For now, we only want to do custom handling for parameter modulation events
    return type == CLAP_EVENT_PARAM_MOD;
}

void CLAPProcessorExtensions::handleEventDirect (const clap_event_header_t* event, int /*sampleOffset*/)
{
    if (event->space_id != CLAP_CORE_EVENT_SPACE_ID || event->type != CLAP_EVENT_PARAM_MOD)
    {
        // we should not be receiving this event!
        jassertfalse;
        return;
    }

    // handle parameter modulation here...
    auto paramModEvent = reinterpret_cast<const clap_event_param_mod*> (event);
    auto* baseParameter = static_cast<juce::AudioProcessorParameter*> (paramModEvent->cookie);

#if JUCE_DEBUG
    {
        const auto parameterFound = modulatableParameters.find (baseParameter) != modulatableParameters.end();
        jassert (parameterFound); // parameter was not found in the list of modulatable parameters!
    }
#endif

    auto modulatableParameter = modulatableParameters[baseParameter];
    if (paramModEvent->note_id >= 0)
    {
        if (modulatableParameter->supportsPolyphonicModulation())
            modulatableParameter->applyPolyphonicModulation (paramModEvent->note_id, paramModEvent->key, paramModEvent->channel, paramModEvent->amount);
    }
    else
    {
        if (modulatableParameter->supportsMonophonicModulation())
            modulatableParameter->applyMonophonicModulation (paramModEvent->amount);
    }
}
} // namespace chowdsp::CLAPExtensions
