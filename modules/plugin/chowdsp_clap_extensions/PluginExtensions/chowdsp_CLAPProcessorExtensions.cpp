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

clap_process_status CLAPProcessorExtensions::clap_direct_process (const clap_process* process) noexcept
{
    const auto numSamples = (int) process->frames_count;
    auto events = process->in_events;
    auto numEvents = (int) events->size (events);
    int currentEvent = 0;
    int nextEventTime = numSamples;

    if (numEvents > 0)
    {
        auto event = events->get (events, 0);
        nextEventTime = (int) event->time;
    }

    // We process in place so...
    static constexpr uint32_t maxBuses = 128;
    std::array<float*, maxBuses> busses {};
    busses.fill (nullptr);
    juce::MidiBuffer midiBuffer;

    static constexpr int smallestBlockSize = 64;
    for (int n = 0; n < numSamples;)
    {
        const auto numSamplesToProcess = (numSamples - n >= smallestBlockSize) ? juce::jmax (nextEventTime - n, smallestBlockSize) // process until next event, but no smaller than smallest block size
                                                                               : (numSamples - n); // process a few leftover samples

        while (nextEventTime < n + numSamplesToProcess && currentEvent < numEvents)
        {
            auto event = events->get (events, (uint32_t) currentEvent);
            process_clap_event (event, midiBuffer, n);

            currentEvent++;
            nextEventTime = (currentEvent < numEvents) ? (int) events->get (events, (uint32_t) currentEvent)->time : numSamples;
        }

        uint32_t outputChannels = 0;
        for (uint32_t idx = 0; idx < process->audio_outputs_count && outputChannels < maxBuses; ++idx)
        {
            for (uint32_t ch = 0; ch < process->audio_outputs[idx].channel_count; ++ch)
            {
                busses[outputChannels] = process->audio_outputs[idx].data32[ch] + n;
                outputChannels++;
            }
        }

        uint32_t inputChannels = 0;
        for (uint32_t idx = 0; idx < process->audio_inputs_count && inputChannels < maxBuses; ++idx)
        {
            for (uint32_t ch = 0; ch < process->audio_inputs[idx].channel_count; ++ch)
            {
                auto* ic = process->audio_inputs[idx].data32[ch] + n;
                if (inputChannels < outputChannels)
                {
                    if (ic == busses[inputChannels])
                    {
                        // The buffers overlap - no need to do anything
                    }
                    else
                    {
                        juce::FloatVectorOperations::copy (busses[inputChannels], ic, numSamplesToProcess);
                    }
                }
                else
                {
                    busses[inputChannels] = ic;
                }
                inputChannels++;
            }
        }

        auto totalChans = juce::jmax (inputChannels, outputChannels);
        juce::AudioBuffer<float> buffer (busses.data(), (int) totalChans, numSamplesToProcess);

        // @TODO: call processBlockBypassed when appropriate
        processor->processBlock (buffer, midiBuffer);

        midiBuffer.clear();
        n += numSamplesToProcess;
    }

    // process any leftover events
    for (; currentEvent < numEvents; ++currentEvent)
    {
        auto event = events->get (events, (uint32_t) currentEvent);
        process_clap_event (event, midiBuffer, 0);
    }

    return CLAP_PROCESS_CONTINUE;
}

void CLAPProcessorExtensions::process_clap_event (const clap_event_header_t* event, juce::MidiBuffer& midiBuffer, int sampleOffset)
{
    if (event->space_id != CLAP_CORE_EVENT_SPACE_ID)
        return;

    switch (event->type)
    {
        case CLAP_EVENT_NOTE_ON:
        {
            auto noteEvent = reinterpret_cast<const clap_event_note*> (event);

            midiBuffer.addEvent (juce::MidiMessage::noteOn (noteEvent->channel + 1,
                                                            noteEvent->key,
                                                            (float)
                                                                noteEvent->velocity),
                                 (int) noteEvent->header.time - sampleOffset);
        }
        break;
        case CLAP_EVENT_NOTE_OFF:
        {
            auto noteEvent = reinterpret_cast<const clap_event_note*> (event);

            midiBuffer.addEvent (juce::MidiMessage::noteOff (noteEvent->channel + 1,
                                                             noteEvent->key,
                                                             (float)
                                                                 noteEvent->velocity),
                                 (int) noteEvent->header.time - sampleOffset);
        }
        break;
        case CLAP_EVENT_MIDI:
        {
            auto midiEvent = reinterpret_cast<const clap_event_midi*> (event);
            midiBuffer.addEvent (juce::MidiMessage (midiEvent->data[0],
                                                    midiEvent->data[1],
                                                    midiEvent->data[2],
                                                    (double) midiEvent->header.time - sampleOffset),
                                 (int) midiEvent->header.time - sampleOffset);
        }
        break;
        case CLAP_EVENT_PARAM_VALUE:
        {
            auto paramEvent = reinterpret_cast<const clap_event_param_value*> (event);
            auto juceParameter = static_cast<juce::AudioProcessorParameter*> (paramEvent->cookie);

            if (juceParameter->getValue() == (float) paramEvent->value)
                return;

            juceParameter->setValueNotifyingHost ((float) paramEvent->value);
        }
        break;
        case CLAP_EVENT_PARAM_MOD:
        {
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
                if (modulatableParameter->supportsMonophonicModulation())
                    modulatableParameter->applyPolyphonicModulation (paramModEvent->note_id, paramModEvent->key, paramModEvent->channel, paramModEvent->amount);
            }
            else
            {
                if (modulatableParameter->supportsMonophonicModulation())
                    modulatableParameter->applyMonophonicModulation (paramModEvent->amount);
            }
        }
        break;
        default:
            break;
    }
}
} // namespace chowdsp::CLAPExtensions
