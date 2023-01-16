#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

namespace
{
constexpr double _sampleRate = 48000.0;
constexpr int _blockSize = 512;
constexpr int _numChannels = 2;
} // namespace

struct TestPlugin : public chowdsp::PluginBase<TestPlugin>
{
    TestPlugin() : oversampling (vts) {}

    static void addParameters (Parameters& params)
    {
        using OSFactor = chowdsp::VariableOversampling<float>::OSFactor;
        using OSMode = chowdsp::VariableOversampling<float>::OSMode;
        decltype (oversampling)::createParameterLayout (params, OSFactor::TwoX, OSMode::MinPhase, 100);
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        oversampling.prepareToPlay (sampleRate, samplesPerBlock, _numChannels);
    }

    void releaseResources() override { oversampling.reset(); }

    void processAudioBlock (juce::AudioBuffer<float>& buffer) override
    {
        oversampling.updateOSFactor();

        auto&& block = juce::dsp::AudioBlock<float> { buffer };
        auto osBlock = oversampling.processSamplesUp (block);

        lastOSBlockSize = (int) osBlock.getNumSamples();

        oversampling.processSamplesDown (block);
    }

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

    auto& getVTS() { return vts; }

    int lastOSBlockSize = 0;

    chowdsp::VariableOversampling<float> oversampling;
};

TEST_CASE ("Variable Oversampling Test", "[dsp][resampling]")
{
    SECTION ("Real-Time Oversampling Test")
    {
        TestPlugin testPlugin;
        testPlugin.prepareToPlay (_sampleRate, _blockSize);

        auto checkOSFactor = [&] (int expectedFactor, const juce::String& message)
        {
            juce::AudioBuffer<float> buffer (_numChannels, _blockSize);
            testPlugin.processAudioBlock (buffer);
            testPlugin.releaseResources();

            REQUIRE_MESSAGE (testPlugin.oversampling.getOSFactor() == expectedFactor, message);
            REQUIRE_MESSAGE (testPlugin.lastOSBlockSize == expectedFactor * _blockSize, message);
        };

        auto& vts = testPlugin.getVTS();
        const juce::String& paramPrefix = "os";
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

    SECTION ("Offline Oversampling Test")
    {
        TestPlugin testPlugin;
        testPlugin.setNonRealtime (true);
        testPlugin.prepareToPlay (_sampleRate, _blockSize);

        auto checkOSFactor = [&] (int expectedFactor, const juce::String& message)
        {
            juce::AudioBuffer<float> buffer (_numChannels, _blockSize);
            testPlugin.processAudioBlock (buffer);
            REQUIRE_MESSAGE (testPlugin.lastOSBlockSize == expectedFactor * _blockSize, message);
        };

        auto& vts = testPlugin.getVTS();
        const juce::String& paramPrefix = "os";
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

    SECTION ("Latency Test")
    {
        TestPlugin testPlugin;
        testPlugin.prepareToPlay (_sampleRate, _blockSize);

        auto checkLatency = [&] (const juce::String& message)
        {
            juce::AudioBuffer<float> buffer (1, _blockSize);
            buffer.clear();
            buffer.setSample (0, 0, 1.0f);
            testPlugin.processAudioBlock (buffer);
            testPlugin.releaseResources();

            auto* outData = buffer.getReadPointer (0);
            auto maxElement = std::max_element (outData, &outData[_blockSize], [] (auto a, auto b)
                                                { return abs (a) < abs (b); });
            auto actualLatencySamples = std::distance (outData, maxElement);

            auto expLatencySamples = testPlugin.oversampling.getLatencySamples();
            REQUIRE_MESSAGE ((float) actualLatencySamples == Catch::Approx { expLatencySamples }.margin (1.1f), message);

            auto actualLatencyMs = ((float) actualLatencySamples / (float) _sampleRate) * 1000.0f;
            auto expLatencyMs = testPlugin.oversampling.getLatencyMilliseconds();
            REQUIRE_MESSAGE (actualLatencyMs == Catch::Approx { expLatencyMs }.margin (0.1f), message);
        };

        auto& vts = testPlugin.getVTS();
        const juce::String& paramPrefix = "os";
        auto* osParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_factor"));
        auto* osModeParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_mode"));

        for (auto osModeVal : { 0, 1 })
        {
            *osModeParam = osModeVal;
            for (auto osParamVal : { 0, 1, 2, 3, 4 })
            {
                *osParam = osParamVal;
                checkLatency ("Incorrect latency! Factor = " + juce::String (std::pow (2, osParamVal)) + ", Mode = " + juce::String (osModeVal));
            }
        }
    }
}
