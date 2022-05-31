#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

#include "PrototypeEQ.h"

/** Example plugin to demonstrate the use of chowdsp::EQProcessor and chowdsp::LinearPhaseEQ */
class SimpleEQPlugin : public chowdsp::PluginBase<SimpleEQPlugin>,
                       private juce::AudioProcessorValueTreeState::Listener
{
public:
    SimpleEQPlugin();
    ~SimpleEQPlugin() override;

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

    EQParams getEQParams() const;
    void loadEQParams (const EQParams& params);

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    void setEQParams();

    std::array<std::atomic<float>*, EQParams::numBands> bandFreqHz {};
    std::array<std::atomic<float>*, EQParams::numBands> bandQ {};
    std::array<std::atomic<float>*, EQParams::numBands> bandGainDB {};
    std::array<std::atomic<float>*, EQParams::numBands> bandType {};
    std::array<std::atomic<float>*, EQParams::numBands> bandOnOff {};
    std::atomic<float>* linPhaseModeOn = nullptr;

    PrototypeEQ protoEQ; // the regular EQ
    chowdsp::EQ::LinearPhaseEQ<PrototypeEQ> linPhaseEQ; // the linear phase EQ

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQPlugin)
};
