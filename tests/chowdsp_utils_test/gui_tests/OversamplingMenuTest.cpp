#include <TimedUnitTest.h>

struct TestPlugin2 : public chowdsp::PluginBase<TestPlugin2>
{
    TestPlugin2() : oversampling (vts) {}

    static void addParameters (Parameters& params)
    {
        using OSFactor = chowdsp::VariableOversampling<float>::OSFactor;
        using OSMode = chowdsp::VariableOversampling<float>::OSMode;
        decltype (oversampling)::createParameterLayout (params, OSFactor::TwoX, OSMode::MinPhase);
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override { oversampling.prepareToPlay (sampleRate, samplesPerBlock, 2); }
    void releaseResources() override {}
    void processAudioBlock (AudioBuffer<float>&) override {}

    auto& getVTS() { return vts; }

    chowdsp::VariableOversampling<float> oversampling;
};

struct TestPlugin3 : public chowdsp::PluginBase<TestPlugin3>
{
    TestPlugin3() : oversampling (vts) {}

    static void addParameters (Parameters& params)
    {
        using OSFactor = chowdsp::VariableOversampling<float>::OSFactor;
        using OSMode = chowdsp::VariableOversampling<float>::OSMode;
        decltype (oversampling)::createParameterLayout (params, OSFactor::TwoX, OSMode::MinPhase, false);
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override { oversampling.prepareToPlay (sampleRate, samplesPerBlock, 2); }
    void releaseResources() override {}
    void processAudioBlock (AudioBuffer<float>&) override {}

    auto& getVTS() { return vts; }

    chowdsp::VariableOversampling<float> oversampling;
};

class OversamplingMenuTest : public TimedUnitTest
{
public:
    OversamplingMenuTest() : TimedUnitTest ("Oversampling Menu Test") {}

    void withOfflineOptionsTest()
    {
        TestPlugin2 plugin;
        plugin.prepareToPlay (48000.0f, 512);
        chowdsp::OversamplingMenu<chowdsp::VariableOversampling<float>> menu (plugin.oversampling, plugin.getVTS());

        auto& vts = plugin.getVTS();
        const String& paramPrefix = "os";
        auto* osOfflineSameParam = dynamic_cast<juce::AudioParameterBool*> (vts.getParameter (paramPrefix + "_render_like_realtime"));
        *osOfflineSameParam = false;

        expectEquals (menu.getRootMenu()->getNumItems(), 11, "Menu has the inccorect number of items!");
    }

    void withoutOfflineOptionsTest()
    {
        TestPlugin3 plugin;
        plugin.prepareToPlay (48000.0f, 512);
        chowdsp::OversamplingMenu<chowdsp::VariableOversampling<float>> menu (plugin.oversampling, plugin.getVTS());
        menu.updateColours();

        expectEquals (menu.getRootMenu()->getNumItems(), 10, "Menu has the inccorect number of items!");
    }

    void beforePrepareTest()
    {
        TestPlugin3 plugin;
        chowdsp::OversamplingMenu<chowdsp::VariableOversampling<float>> menu (plugin.oversampling, plugin.getVTS());
        menu.updateColours();

        expectEquals (menu.getRootMenu()->getNumItems(), 0, "Menu has the inccorect number of items!");
    }

    void runTestTimed() override
    {
        beginTest ("With Offline Options Test");
        withOfflineOptionsTest();

        beginTest ("Without Offline Options Test");
        withoutOfflineOptionsTest();

        beginTest ("Before Preparation Test");
        beforePrepareTest();
    }
};

static OversamplingMenuTest oversamplingMenuTest;
