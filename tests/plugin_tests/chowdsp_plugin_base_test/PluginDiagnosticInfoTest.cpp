#include <DummyPlugin.h>
#include <TimedUnitTest.h>

class PluginDiagnosticInfoTest : public TimedUnitTest
{
public:
    PluginDiagnosticInfoTest() : TimedUnitTest ("Plugin Diagnostic Info Test") {}

    void runTestTimed() override
    {
        static constexpr auto sampleRate = 44100.0;
        static constexpr int blockSize = 256;

        DummyPlugin plugin;
        plugin.prepareToPlay (sampleRate, blockSize);
        const auto diagString = chowdsp::PluginDiagnosticInfo::getDiagnosticsString (plugin);
        juce::Logger::writeToLog (diagString);

        beginTest ("Name/Version Test");
        expect (diagString.contains ("Version: DummyPlugin 9.9.9"), "Diag name/version is icnorrect!");

        beginTest ("Sample Rate/Block Size Test");
        expect (diagString.contains ("running at sample rate 44.1 kHz with block size 256"), "Diag sample rate info is incorrect!");
    }
};

static PluginDiagnosticInfoTest pluginDiagnosticInfoTest;
