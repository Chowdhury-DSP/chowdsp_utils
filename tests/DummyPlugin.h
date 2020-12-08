#pragma once

#include <JuceHeader.h>

/** Dummy plugin used for unit tests */
class DummyPlugin : public chowdsp::PluginBase<DummyPlugin>
{
public:
    DummyPlugin() {}

    static void addParameters (Parameters& params)
    {
        params.push_back (std::make_unique<AudioParameterFloat> ("dummy", "Dummy", 0.0f, 1.0f, 0.5f));
    }

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processAudioBlock (AudioBuffer<float>&) override {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DummyPlugin)
};
