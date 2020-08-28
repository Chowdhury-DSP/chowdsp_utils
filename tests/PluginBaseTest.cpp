#include <JuceHeader.h>

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
    void processBlock (AudioBuffer<float>&) override {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DummyPlugin)
};

class PluginBaseTest : public UnitTest
{
public:
    PluginBaseTest() : UnitTest ("Plugin Base Test") {}

    void saveLoadStateTest()
    {
        constexpr float testValue = 0.234f;

        DummyPlugin dummy1;
        dummy1.getParameters()[0]->setValue (testValue);

        MemoryBlock stateBlock;
        dummy1.getStateInformation (stateBlock);

        DummyPlugin dummy2;
        dummy2.setStateInformation (stateBlock.getData(), (int) stateBlock.getSize());
        
        MemoryBlock stateBlock2;
        dummy2.getStateInformation (stateBlock2);

        expect (stateBlock == stateBlock2, "Plugin base did not load correct state!");
    }

    void channelSetTest()
    {
        DummyPlugin dummy;

        AudioProcessor::BusesLayout stLayout { { AudioChannelSet::stereo() }, { AudioChannelSet::stereo() } };
        expect (dummy.isBusesLayoutSupported(stLayout), "Stereo I/O should be supported!");

        AudioProcessor::BusesLayout monoLayout { { AudioChannelSet::mono() }, { AudioChannelSet::mono() } };
        expect (dummy.isBusesLayoutSupported(monoLayout), "Mono I/O should be supported!");

        AudioProcessor::BusesLayout smLayout { { AudioChannelSet::stereo() }, { AudioChannelSet::mono() } };
        expect (! dummy.isBusesLayoutSupported(smLayout), "Stereo in, mono out should NOT be supported!");

        AudioProcessor::BusesLayout msLayout { { AudioChannelSet::mono() }, { AudioChannelSet::stereo() } };
        expect (! dummy.isBusesLayoutSupported(msLayout), "Mono in, stereo out should NOT be supported!");

        AudioProcessor::BusesLayout quadLayout { { AudioChannelSet::quadraphonic() }, { AudioChannelSet::quadraphonic() } };
        expect (! dummy.isBusesLayoutSupported(quadLayout), "More than two channels should NOT be supported!");
    }

    void runTest() override
    {
        beginTest ("Save/Load State Test");
        saveLoadStateTest();

        beginTest ("Buses Layout Test");
        channelSetTest();
    }
};

static PluginBaseTest pbTest;
