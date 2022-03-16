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

    void prepareToPlay (double, int) override {}
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

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processAudioBlock (AudioBuffer<float>&) override {}

    auto& getVTS() { return vts; }

    chowdsp::VariableOversampling<float> oversampling;
};

class OversamplingMenuTest : public TimedUnitTest
{
public:
    OversamplingMenuTest() : TimedUnitTest ("Oversampling Menu Test") {}

    static void withOfflineOptionsTest()
    {
        TestPlugin2 plugin;
        chowdsp::OversamplingMenu<chowdsp::VariableOversampling<float>> menu (plugin.oversampling, plugin.getVTS());

        menu.setBounds (0, 0, 100, 100);
        Graphics g { Image() };
        menu.paint (g);

        auto& vts = plugin.getVTS();
        const String& paramPrefix = "os";
        auto* osOfflineSameParam = dynamic_cast<juce::AudioParameterBool*> (vts.getParameter (paramPrefix + "_render_like_realtime"));
        *osOfflineSameParam = false;
    }

    static void withoutOfflineOptionsTest()
    {
        TestPlugin3 plugin;
        chowdsp::OversamplingMenu<chowdsp::VariableOversampling<float>> menu (plugin.oversampling, plugin.getVTS());

        menu.setBounds (0, 0, 100, 100);
        Graphics g { Image() };
        menu.paint (g);
    }

    void runTestTimed() override
    {
        beginTest ("With Offline Options Test");
        withOfflineOptionsTest();

        beginTest ("Without Offline Options Test");
        withoutOfflineOptionsTest();
    }
};

static OversamplingMenuTest oversamplingMenuTest;
