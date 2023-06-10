#include "PluginEditor.h"

void PluginEditor::LevelDetectorEditor::resized()
{
    auto bounds = getLocalBounds();
    paramsView->setBounds (bounds.removeFromTop (proportionOfHeight (0.5f)));
    viz->setBounds (bounds);
}

void PluginEditor::GainComputerEditor::resized()
{
    auto bounds = getLocalBounds();
    paramsView->setBounds (bounds.removeFromTop (proportionOfHeight (0.5f)));
    plot.setBounds (bounds);
    updatePlot();
}

void PluginEditor::GainComputerEditor::setupPlot (chowdsp::PluginState& state, Params& plugParams)
{
    pluginParams = &plugParams;
    gainComputer.prepare (48000.0, inputBuffer.getNumSamples());
    callbacks += {
        state.addParameterListener (pluginParams->threshold, chowdsp::ParameterListenerThread::MessageThread, [this]
                                    { updatePlot(); }),
        state.addParameterListener (pluginParams->ratio, chowdsp::ParameterListenerThread::MessageThread, [this]
                                    { updatePlot(); }),
        state.addParameterListener (pluginParams->knee, chowdsp::ParameterListenerThread::MessageThread, [this]
                                    { updatePlot(); }),
        state.addParameterListener (pluginParams->architecture, chowdsp::ParameterListenerThread::MessageThread, [this]
                                    { updatePlot(); }),
    };
    for (auto [n, sample] : chowdsp::enumerate (inputBuffer.getWriteSpan (0)))
        sample = juce::Decibels::decibelsToGain (juce::jmap ((float) n,
                                                             0.0f,
                                                             (float) inputBuffer.getNumSamples(),
                                                             plot.params.xMin,
                                                             plot.params.xMax));
    updatePlot();
}

void PluginEditor::GainComputerEditor::updatePlot()
{
    gainComputer.setMode (*magic_enum::enum_index (pluginParams->architecture->get()));
    gainComputer.setThreshold (pluginParams->threshold->get());
    gainComputer.setRatio (pluginParams->ratio->get());
    gainComputer.setKnee (pluginParams->knee->get());
    gainComputer.reset();

    gainComputer.processBlock (inputBuffer, outputBuffer);

    plot.setThreshold (pluginParams->threshold->get());
    plot.updatePlotPath (inputBuffer.getReadSpan (0), outputBuffer.getReadSpan (0));
}

PluginEditor::PluginEditor (ExampleCompressorPlugin& plugin)
    : juce::AudioProcessorEditor (plugin),
      meter (plugin.compressor.gainReductionMeterTask)
{
    auto& params = plugin.getState().params;
    levelDetectorEditor.params.add (params.attack, params.release, params.levelDetectorMode);
    levelDetectorEditor.paramsView.emplace (plugin.getState(), levelDetectorEditor.params);
    levelDetectorEditor.addAndMakeVisible (*levelDetectorEditor.paramsView);
    levelDetectorEditor.viz = &plugin.compressor.levelDetector.levelDetectorViz;
    levelDetectorEditor.addAndMakeVisible (levelDetectorEditor.viz);
    addAndMakeVisible (levelDetectorEditor);

    gainComputerEditor.params.add (params.threshold, params.ratio, params.knee, params.architecture, params.autoMakeup);
    gainComputerEditor.paramsView.emplace (plugin.getState(), gainComputerEditor.params);
    gainComputerEditor.addAndMakeVisible (*gainComputerEditor.paramsView);
    gainComputerEditor.addAndMakeVisible (gainComputerEditor.plot);
    gainComputerEditor.setupPlot (plugin.getState(), params);
    addAndMakeVisible (gainComputerEditor);

    addAndMakeVisible (meter);

    setSize (900, 400);
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds();
    levelDetectorEditor.setBounds (bounds.removeFromLeft (proportionOfWidth (0.46f)));
    gainComputerEditor.setBounds (bounds.removeFromLeft (proportionOfWidth (0.46f)));
    meter.setBounds (bounds);
}
