#include <DummyPlugin.h>
#include <test_utils.h>
#include <TimedUnitTest.h>

/** Unit tests for chowdsp::PluginBase. Tests functionality:
 *   - saving/loading parameter state
 *   - channel layouts supported
 */
class PluginBaseTest : public TimedUnitTest
{
public:
    PluginBaseTest() : TimedUnitTest ("Plugin Base Test") {}

    void saveLoadStateTest()
    {
        constexpr float testValue = 0.234f;

        DummyPlugin dummy1;
        dummy1.getParameters()[0]->setValue (testValue);

        juce::MemoryBlock stateBlock;
        dummy1.getStateInformation (stateBlock);

        DummyPlugin dummy2;
        dummy2.setStateInformation (stateBlock.getData(), (int) stateBlock.getSize());

        juce::MemoryBlock stateBlock2;
        dummy2.getStateInformation (stateBlock2);

        expect (stateBlock == stateBlock2, "Plugin base did not load correct state!");
    }

    void channelSetTest()
    {
        DummyPlugin dummy;

        juce::AudioProcessor::BusesLayout stLayout { { juce::AudioChannelSet::stereo() }, { juce::AudioChannelSet::stereo() } };
        expect (dummy.isBusesLayoutSupported (stLayout), "Stereo I/O should be supported!");

        juce::AudioProcessor::BusesLayout monoLayout { { juce::AudioChannelSet::mono() }, { juce::AudioChannelSet::mono() } };
        expect (dummy.isBusesLayoutSupported (monoLayout), "Mono I/O should be supported!");

        juce::AudioProcessor::BusesLayout smLayout { { juce::AudioChannelSet::stereo() }, { juce::AudioChannelSet::mono() } };
        expect (! dummy.isBusesLayoutSupported (smLayout), "Stereo in, mono out should NOT be supported!");

        juce::AudioProcessor::BusesLayout msLayout { { juce::AudioChannelSet::mono() }, { juce::AudioChannelSet::stereo() } };
        expect (! dummy.isBusesLayoutSupported (msLayout), "Mono in, stereo out should NOT be supported!");

        juce::AudioProcessor::BusesLayout quadLayout { { juce::AudioChannelSet::quadraphonic() }, { juce::AudioChannelSet::quadraphonic() } };
        expect (! dummy.isBusesLayoutSupported (quadLayout), "More than two channels should NOT be supported!");
    }

    void propertiesTest()
    {
        DummyPlugin dummy1;

        expectEquals (dummy1.getName(), juce::String ("DummyPlugin"), "Plugin name incorrect!");

        bool midiBehaviour = dummy1.acceptsMidi() || dummy1.producesMidi() || dummy1.isMidiEffect();
        expect (! midiBehaviour, "MIDI behaviour incorrect!");

        expectEquals (dummy1.getTailLengthSeconds(), 0.0, "Tail length incorrect!");

        expect (dummy1.getNumPrograms() > 0, "Num programs is to low!");
        dummy1.setCurrentProgram (0);
        expectEquals (dummy1.getCurrentProgram(), 0, "Current program is incorrect!");
        dummy1.changeProgramName (0, "");
        expectEquals (dummy1.getProgramName (0), juce::String(), "Program name incorrect");
    }

    void guiTest()
    {
        DummyPlugin dummy1;

        if (dummy1.hasEditor())
        {
            std::unique_ptr<juce::AudioProcessorEditor> editor (dummy1.createEditor());

            expect (editor != nullptr, "Editor should not be nullptr!");

            if (editor != nullptr)
            {
                editor->setSize (700, 700); // make editor larger
                dummy1.editorBeingDeleted (editor.get());
            }
        }
        else
        {
            expect (false, "Dummy plugin has no editor!");
        }
    }

    void processTest()
    {
        DummyPlugin dummy1;
        constexpr int blockSize = 1024;
        dummy1.prepareToPlay (48000.0, blockSize);

        // float test
        {
            juce::AudioBuffer<float> buffer (2, blockSize);
            buffer.clear();
            juce::MidiBuffer midi;

            dummy1.processBlock (buffer, midi);
            expectLessThan (buffer.getMagnitude (0, blockSize), 1.0e-3f, "Buffer should be empty!");
        }

        // double test
        {
            juce::AudioBuffer<double> buffer (2, blockSize);
            buffer.clear();
            juce::MidiBuffer midi;

            dummy1.processBlock (buffer, midi);
            expectLessThan (buffer.getMagnitude (0, blockSize), 1.0e-6, "Buffer should be empty!");
        }
    }

    void programInterfaceTest()
    {
        using namespace test_utils;

        DummyPlugin dummy { true };
        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        auto& presetMgr = dummy.getPresetManager();
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        ScopedFile presetFile1 ("preset_path/test1.preset");
        presetMgr.saveUserPreset (presetFile1.file);

        ScopedFile presetFile2 ("preset_path/test2.preset");
        presetMgr.saveUserPreset (presetFile2.file);

        expectEquals (dummy.getNumPrograms(), presetMgr.getNumPresets(), "Num presets incorrect!");
        expectEquals (dummy.getCurrentProgram(), presetMgr.getCurrentPresetIndex(), "Initial program index incorrect!");

        dummy.setCurrentProgram (0);
        expectEquals (dummy.getCurrentProgram(), presetMgr.getCurrentPresetIndex(), "New program index incorrect!");

        expectEquals (dummy.getProgramName (0), presetMgr.getPresetName (0), "Program name incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void programInterfaceOffTest()
    {
        using namespace test_utils;

        DummyPlugin dummy { true };
        dummy.setUsePresetManagerForPluginInterface (false);

        ScopedFile presetPath ("preset_path");
        presetPath.file.createDirectory();

        auto& presetMgr = dummy.getPresetManager();
        presetMgr.setUserPresetConfigFile ("preset_config.txt");
        presetMgr.setUserPresetPath (presetPath.file);

        ScopedFile presetFile1 ("preset_path/test1.preset");
        presetMgr.saveUserPreset (presetFile1.file);

        ScopedFile presetFile2 ("preset_path/test2.preset");
        presetMgr.saveUserPreset (presetFile2.file);

        expectEquals (dummy.getNumPrograms(), 1, "Num presets incorrect!");
        expectEquals (dummy.getCurrentProgram(), 0, "Initial program index incorrect!");

        dummy.setCurrentProgram (0);
        expectEquals (dummy.getCurrentProgram(), 0, "New program index incorrect!");

        expectEquals (dummy.getProgramName (0), juce::String(), "Program name incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void runTestTimed() override
    {
        beginTest ("Save/Load State Test");
        saveLoadStateTest();

        beginTest ("Buses Layout Test");
        channelSetTest();

        beginTest ("Properties Test");
        propertiesTest();

        beginTest ("GUI Test");
        guiTest();

        beginTest ("Process Test");
        processTest();

        beginTest ("Program Interface Test");
        programInterfaceTest();

        beginTest ("Program Interface Off Test");
        programInterfaceOffTest();
    }
};

static PluginBaseTest pbTest;
