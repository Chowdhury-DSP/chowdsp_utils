#include "PluginEditor.h"

struct LevelDetectorEditor : juce::Component
{
    LevelDetectorEditor (chowdsp::PluginState& state,
                         Params& pluginParams,
                         chow_comp::LevelDetectorVisualizer& levelViz,
                         chowdsp::ComponentArena<>& arena)
        : params { arena.allocator, "Level Detector Params", false },
          viz { &levelViz }
    {
        params.add (pluginParams.attack, pluginParams.release, pluginParams.levelDetectorMode);

        paramsView = arena.allocate<chowdsp::ParametersView> (state, params);
        addAndMakeVisible (paramsView);

        addAndMakeVisible (viz);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        paramsView->setBounds (bounds.removeFromTop (proportionOfHeight (0.5f)));
        viz->setBounds (bounds);
    }

    chowdsp::ParamHolder params;
    chowdsp::ParametersView* paramsView {};
    juce::Component* viz {};
};

struct GainComputerEditor : juce::Component
{
    GainComputerEditor (chowdsp::PluginState& state,
                        Params& plugParams,
                        chowdsp::ComponentArena<>& arena)
        : params { arena.allocator, "Gain Computer Params", false }
    {
        params.add (plugParams.threshold, plugParams.ratio, plugParams.knee, plugParams.architecture, plugParams.autoMakeup);

        paramsView = arena.allocate<chowdsp::ParametersView> (state, params);
        addAndMakeVisible (paramsView);

        addAndMakeVisible (plot);
        setupPlot (state, plugParams);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        paramsView->setBounds (bounds.removeFromTop (proportionOfHeight (0.5f)));
        plot.setBounds (bounds);
        updatePlot();
    }

    void setupPlot (chowdsp::PluginState& state, Params& plugParams)
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

    void updatePlot()
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

    chowdsp::ParamHolder params;
    chowdsp::ParametersView* paramsView {};

    Params* pluginParams = nullptr;
    GainComputer gainComputer;
    chow_comp::GainComputerPlot plot;

    chowdsp::ScopedCallbackList callbacks;
    static constexpr int plotNumSamples = 128;
    chowdsp::StaticBuffer<float, 1, plotNumSamples> inputBuffer { 1, plotNumSamples };
    chowdsp::StaticBuffer<float, 1, plotNumSamples> outputBuffer { 1, plotNumSamples };
};

PluginEditor::PluginEditor (ExampleCompressorPlugin& plugin)
    : juce::AudioProcessorEditor (plugin)
{
    levelDetectorEditor = arena.allocate<LevelDetectorEditor> (plugin.getState(),
                                                               plugin.getState().params,
                                                               plugin.compressor.levelDetector.levelDetectorViz,
                                                               arena);
    addAndMakeVisible (levelDetectorEditor);

    gainComputerEditor = arena.allocate<GainComputerEditor> (plugin.getState(),
                                                             plugin.getState().params,
                                                             arena);
    addAndMakeVisible (gainComputerEditor);

    meter = arena.allocate<chow_comp::GainReductionMeter> (plugin.compressor.gainReductionMeterTask);
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
    levelDetectorEditor->setBounds (bounds.removeFromLeft (proportionOfWidth (0.46f)));
    gainComputerEditor->setBounds (bounds.removeFromLeft (proportionOfWidth (0.46f)));
    meter->setBounds (bounds);
}
