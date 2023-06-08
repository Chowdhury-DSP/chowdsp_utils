#pragma once

#include "ExampleCompressor.h"
#include <chowdsp_gui/chowdsp_gui.h>

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (ExampleCompressorPlugin& plugin);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    struct LevelDetectorEditor : juce::Component
    {
        void resized() override;

        chowdsp::ParamHolder params { "Level Detector Params", false };
        std::optional<chowdsp::ParametersView> paramsView;
        juce::Component* viz;
    } levelDetectorEditor;

    struct GainComputerEditor : juce::Component
    {
        void resized() override;
        void setupPlot (chowdsp::PluginState& state, Params& pluginParams);
        void updatePlot();

        chowdsp::ParamHolder params { "Gain Computer Params", false };
        std::optional<chowdsp::ParametersView> paramsView;

        Params* pluginParams = nullptr;
        GainComputer gainComputer;
        chow_comp::GainComputerPlot plot;

        chowdsp::ScopedCallbackList callbacks;
        static constexpr int plotNumSamples = 128;
        chowdsp::StaticBuffer<float, 1, plotNumSamples> inputBuffer { 1, plotNumSamples };
        chowdsp::StaticBuffer<float, 1, plotNumSamples> outputBuffer { 1, plotNumSamples };
    } gainComputerEditor;

    chow_comp::GainReductionMeter meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
