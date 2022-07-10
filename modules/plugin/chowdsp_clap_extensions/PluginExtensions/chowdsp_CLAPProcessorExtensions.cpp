#include "chowdsp_CLAPProcessorExtensions.h"

namespace chowdsp::CLAPExtensions
{
void CLAPProcessorExtensions::initialise (juce::AudioProcessor& baseProcessor)
{
    juce::ignoreUnused (baseProcessor);
    //    processor = &baseProcessor;
    //    jassert (processor != nullptr); // initialized with a NULL processor?
    //
    //    for (auto* param : processor->getParameters())
    //    {
    //        if (auto* modulatableParameter = dynamic_cast<ParamUtils::ModParameterMixin*> (param))
    //            modulatableParameters[param] = modulatableParameter;
    //    }
}

bool CLAPProcessorExtensions::supportsDirectEvent (uint16_t space_id, uint16_t type)
{
    if (space_id != CLAP_CORE_EVENT_SPACE_ID)
        return false;

    // For now, we only want to do custom handling for parameter modulation events
    return type == CLAP_EVENT_PARAM_MOD;
}

void CLAPProcessorExtensions::handleDirectEvent (const clap_event_header_t* event, int /*sampleOffset*/)
{
    if (event->space_id != CLAP_CORE_EVENT_SPACE_ID || event->type != CLAP_EVENT_PARAM_MOD)
    {
        // we should not be receiving this event!
        jassertfalse;
        return;
    }

    // handle parameter modulation here...
    auto paramModEvent = reinterpret_cast<const clap_event_param_mod*> (event);
    auto* paramVariant = static_cast<JUCEParameterVariant*> (paramModEvent->cookie);

    if (auto* modulatableParameter = paramVariant->clapExtParameter)
    {
        if (paramModEvent->note_id >= 0)
        {
            if (! modulatableParameter->supportsPolyphonicModulation())
            {
                // The host is misbehaving: it should know this parameter doesn't support
                // mono modulation and not have sent this event in the first place!
                jassertfalse;
                return;
            }

            modulatableParameter->applyPolyphonicModulation (paramModEvent->note_id, paramModEvent->port_index, paramModEvent->channel, paramModEvent->key, paramModEvent->amount);
        }
        else
        {
            if (! modulatableParameter->supportsMonophonicModulation())
            {
                // The host is misbehaving: it should know this parameter doesn't support
                // poly modulation and not have sent this event in the first place!
                jassertfalse;
                return;
            }

            modulatableParameter->applyMonophonicModulation (paramModEvent->amount);
        }
    }
}
} // namespace chowdsp::CLAPExtensions
