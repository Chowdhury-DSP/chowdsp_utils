#include <juce_dsp/juce_dsp.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

class TestPlugin : public chowdsp::PluginBase<TestPlugin>
{
public:
    TestPlugin() = default;

    // Plugin class has an addParameters method, so this should compile
    static void addParameters (Parameters&) {}

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>&) override {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
};

int main()
{
    TestPlugin plugin;
    return 0;
}
