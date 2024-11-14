#include "SimpleEQPlugin.h"
#include "PluginEditor.h"

SimpleEQPlugin::SimpleEQPlugin()
    : eqParamsHandles (state.params.eqParams.eqParams)
{
    // let's check that we set up our EQ band choices correctly!
    using EQFilterChoices = types_list::TupleList<EQBand<float>::FilterChoicesTuple>;
    jassert (EQFilterChoices::count == (size_t) Params::bandTypeChoices.size());
    types_list::forEach<EQFilterChoices> (
        [] (auto typeIndex)
        {
            using TypeAtIndex = EQFilterChoices::AtIndex<typeIndex>;
            juce::Logger::writeToLog (chowdsp::toString (NAMEOF_TYPE (TypeAtIndex))
                                      + ", " + Params::bandTypeChoices[static_cast<int> (typeIndex)]);
        });

    linPhaseEQ.updatePrototypeEQParameters = [] (auto& eq, auto& eqParams)
    { eq.setParameters (eqParams); };

    linPhaseModeChangeCallback = state.addParameterListener (*state.params.linPhaseMode,
                                                             chowdsp::ParameterListenerThread::MessageThread,
                                                             [this]
                                                             {
                                                                 setLatencySamples (state.params.linPhaseMode.get() ? linPhaseEQ.getLatencySamples() : 0);
                                                             });
}

void SimpleEQPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto&& eqParams = getEQParams();
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumInputChannels() };

    protoEQ.setParameters (eqParams);
    protoEQ.prepare (spec);

    linPhaseEQ.prepare (spec, getEQParams()); // prepare the linear phase EQ with some initial parameters

    // make sure the reported latency is up-to-date
    setLatencySamples (state.params.linPhaseMode.get() ? linPhaseEQ.getLatencySamples() : 0);
}

PrototypeEQ::Params SimpleEQPlugin::getEQParams() const
{
    return EQParams::getEQParameters (eqParamsHandles);
}

void SimpleEQPlugin::loadEQParams (const PrototypeEQ::Params& params)
{
    EQParams::loadEQParameters (params, state.params.eqParams);
}

void SimpleEQPlugin::setEQParams()
{
    const auto&& eqParams = getEQParams();
    protoEQ.setParameters (eqParams);
    linPhaseEQ.setParameters (eqParams);
}

void SimpleEQPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    // update the linear phase EQ parameters. This operation is safe to do from
    // any thread, but we'll do it on the audio thread here.
    setEQParams();

    if (state.params.linPhaseMode->get())
    {
        // Linear phase mode is on: processing the linear phase EQ here!
        linPhaseEQ.processBlock (buffer);
    }
    else
    {
        // Not in linear phase mode, so just process the regular EQ
        protoEQ.processBlock (buffer);
    }
}

juce::AudioProcessorEditor* SimpleEQPlugin::createEditor()
{
    return new PluginEditor (*this);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQPlugin();
}
