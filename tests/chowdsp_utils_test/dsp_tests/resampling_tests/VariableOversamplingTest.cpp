#include <TimedUnitTest.h>

struct TestPlugin : public chowdsp::PluginBase<TestPlugin>
{
    TestPlugin() : oversampling (vts) {}

    static void addParameters (Parameters& params)
    {
        using OSFactor = chowdsp::VariableOversampling<float>::OSFactor;
        using OSMode = chowdsp::VariableOversampling<float>::OSMode;
        decltype (oversampling)::createParameterLayout (params, OSFactor::TwoX, OSMode::MinPhase);
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        oversampling.prepareToPlay (sampleRate, samplesPerBlock);
    }

    void releaseResources() override { oversampling.reset(); }

    void processAudioBlock (AudioBuffer<float>& buffer) override
    {
        oversampling.updateOSFactor();

        auto&& block = dsp::AudioBlock<float> { buffer };
        auto osBlock = oversampling.processSamplesUp (block);

        lastOSBlockSize = (int) osBlock.getNumSamples();

        oversampling.processSamplesDown (block);
    }

    auto& getVTS() { return vts; }

    int lastOSBlockSize = 0;

    chowdsp::VariableOversampling<float> oversampling;
};

namespace
{
constexpr double _sampleRate = 48000.0;
constexpr int _blockSize = 512;
} // namespace

class VariableOversamplingTest : public TimedUnitTest
{
public:
    VariableOversamplingTest() : TimedUnitTest ("Variable Oversampling Test") {}

    void realTimeTest()
    {
        TestPlugin testPlugin;
        testPlugin.prepareToPlay (_sampleRate, _blockSize);

        auto checkOSFactor = [&] (int expectedFactor, const String& message)
        {
            AudioBuffer<float> buffer (2, _blockSize);
            testPlugin.processAudioBlock (buffer);
            testPlugin.releaseResources();

            expectEquals (testPlugin.oversampling.getOSFactor(), expectedFactor, message);
            expectEquals (testPlugin.lastOSBlockSize, expectedFactor * _blockSize, message);
        };

        auto& vts = testPlugin.getVTS();
        const String& paramPrefix = "os";
        auto* osParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_factor"));
        auto* osModeParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_mode"));
        auto* osOfflineParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_render_factor"));

        // Check initial oversampling factor
        checkOSFactor (2, "Initial OS factor incorrect!");

        // Check that changing offline param does nothing
        *osOfflineParam = 4;
        checkOSFactor (2, "OS factor after changing offline param is incorrect!");

        // Check that changing OS factor works
        *osParam = 3;
        checkOSFactor (8, "New OS factor incorrect!");

        // Check that changing OS mode does not affect factor
        *osModeParam = 1;
        checkOSFactor (8, "OS factor after changin OS mode is incorrect!");
    }

    void offlineTest()
    {
        TestPlugin testPlugin;
        testPlugin.setNonRealtime (true);
        testPlugin.prepareToPlay (_sampleRate, _blockSize);

        auto checkOSFactor = [&] (int expectedFactor, const String& message)
        {
            AudioBuffer<float> buffer (2, _blockSize);
            testPlugin.processAudioBlock (buffer);
            expectEquals (testPlugin.lastOSBlockSize, expectedFactor * _blockSize, message);
        };

        auto& vts = testPlugin.getVTS();
        const String& paramPrefix = "os";
        auto* osParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_factor"));
        auto* osOfflineParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_render_factor"));
        auto* osOfflineModeParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_render_mode"));
        auto* osOfflineSameParam = dynamic_cast<juce::AudioParameterBool*> (vts.getParameter (paramPrefix + "_render_like_realtime"));

        *osOfflineSameParam = false;

        // Check initial oversampling factor
        checkOSFactor (2, "Initial OS factor incorrect!");

        // Check that changing real-time param does nothing
        *osParam = 4;
        checkOSFactor (2, "OS factor after changing real-time param is incorrect!");

        // Check that changing OS factor works
        *osOfflineParam = 3;
        checkOSFactor (8, "New OS factor incorrect!");

        // Check that changing OS mode does not affect factor
        *osOfflineModeParam = 1;
        checkOSFactor (8, "OS factor after changin OS mode is incorrect!");

        // Check that "same as real-time" parameter works
        *osOfflineSameParam = true;
        checkOSFactor (16, "OS factor \"same as real-time\" is incorrect!");
    }

    void latencyTest()
    {
        TestPlugin testPlugin;
        testPlugin.prepareToPlay (_sampleRate, _blockSize);

        auto checkLatency = [&] (const String& message)
        {
            AudioBuffer<float> buffer (1, _blockSize);
            buffer.clear();
            buffer.setSample (0, 0, 1.0f);
            testPlugin.processAudioBlock (buffer);
            testPlugin.releaseResources();

            auto* outData = buffer.getReadPointer (0);
            auto maxElement = std::max_element (outData, &outData[_blockSize], [] (auto a, auto b)
                                                { return abs (a) < abs (b); });
            auto actualLatencySamples = std::distance (outData, maxElement);

            auto expLatencySamples = testPlugin.oversampling.getLatencySamples();
            expectWithinAbsoluteError ((float) actualLatencySamples, expLatencySamples, 1.1f, message);
        };

        auto& vts = testPlugin.getVTS();
        const String& paramPrefix = "os";
        auto* osParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_factor"));
        auto* osModeParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_mode"));

        for (auto osModeVal : { 0, 1 })
        {
            *osModeParam = osModeVal;
            for (auto osParamVal : { 0, 1, 2, 3, 4 })
            {
                *osParam = osParamVal;
                checkLatency ("Incorrect latency! Factor = " + String (std::pow (2, osParamVal)) + ", Mode = " + String (osModeVal));
            }
        }
    }

    void runTestTimed() override
    {
        beginTest ("Real-Time Oversampling Test");
        realTimeTest();

        beginTest ("Offline Oversampling Test");
        offlineTest();

        beginTest ("Latency Test");
        latencyTest();
    }
};

static VariableOversamplingTest variableOversamplingTest;
