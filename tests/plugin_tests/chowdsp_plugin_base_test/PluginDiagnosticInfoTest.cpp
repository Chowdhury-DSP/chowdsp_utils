#include <CatchUtils.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

TEST_CASE ("Plugin Diagnostic Info Test", "[plugin]")
{
    static constexpr auto sampleRate = 44100.0;
    static constexpr int blockSize = 256;

    struct Params : chowdsp::ParamHolder
    {
    };

    struct DummyPlugin : chowdsp::PluginBase<chowdsp::PluginStateImpl<Params>>
    {
        void releaseResources() override {}
        void processAudioBlock (juce::AudioBuffer<float>&) override {}
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    };

    DummyPlugin plugin;
    plugin.prepareToPlay (sampleRate, blockSize);
    const auto diagString = chowdsp::PluginDiagnosticInfo::getDiagnosticsString (plugin);
    juce::Logger::writeToLog (diagString);

    REQUIRE_MESSAGE (diagString.contains ("Version: TestPlugin 9.9.9"), "Diag name/version is incorrect!");
    REQUIRE_MESSAGE (diagString.contains ("running at sample rate 44.1 kHz with block size 256"), "Diag sample rate info is incorrect!");
}
