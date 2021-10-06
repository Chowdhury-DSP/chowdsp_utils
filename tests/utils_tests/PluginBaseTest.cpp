#include "DummyPlugin.h"
#include "test_utils.h"

/** Unit tests for chowdsp::PluginBase. Tests functionality:
 *   - saving/loading parameter state
 *   - channel layouts supported
 */
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
        expect (dummy.isBusesLayoutSupported (stLayout), "Stereo I/O should be supported!");

        AudioProcessor::BusesLayout monoLayout { { AudioChannelSet::mono() }, { AudioChannelSet::mono() } };
        expect (dummy.isBusesLayoutSupported (monoLayout), "Mono I/O should be supported!");

        AudioProcessor::BusesLayout smLayout { { AudioChannelSet::stereo() }, { AudioChannelSet::mono() } };
        expect (! dummy.isBusesLayoutSupported (smLayout), "Stereo in, mono out should NOT be supported!");

        AudioProcessor::BusesLayout msLayout { { AudioChannelSet::mono() }, { AudioChannelSet::stereo() } };
        expect (! dummy.isBusesLayoutSupported (msLayout), "Mono in, stereo out should NOT be supported!");

        AudioProcessor::BusesLayout quadLayout { { AudioChannelSet::quadraphonic() }, { AudioChannelSet::quadraphonic() } };
        expect (! dummy.isBusesLayoutSupported (quadLayout), "More than two channels should NOT be supported!");
    }

    void propertiesTest()
    {
        DummyPlugin dummy1;

        expectEquals (dummy1.getName(), String ("DummyPlugin"), "Plugin name incorrect!");

        bool midiBehaviour = dummy1.acceptsMidi() || dummy1.producesMidi() || dummy1.isMidiEffect();
        expect (! midiBehaviour, "MIDI behaviour incorrect!");

        expectEquals (dummy1.getTailLengthSeconds(), 0.0, "Tail length incorrect!");

        expect (dummy1.getNumPrograms() > 0, "Num programs is to low!");
        dummy1.setCurrentProgram (0);
        expectEquals (dummy1.getCurrentProgram(), 0, "Current program is incorrect!");
        dummy1.changeProgramName (0, "");
        expectEquals (dummy1.getProgramName (0), String(), "Program name incorrect");
    }

    void guiTest()
    {
        DummyPlugin dummy1;

        if (dummy1.hasEditor())
        {
            std::unique_ptr<AudioProcessorEditor> editor (dummy1.createEditor());
            editor->setSize (700, 700); // make editor larger
            dummy1.editorBeingDeleted (editor.get());
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
            AudioBuffer<float> buffer (2, blockSize);
            buffer.clear();
            MidiBuffer midi;

            dummy1.processBlock (buffer, midi);
            expectLessThan (buffer.getMagnitude (0, blockSize), 1.0e-3f, "Buffer should be empty!");
        }

        // double test
        {
            AudioBuffer<double> buffer (2, blockSize);
            buffer.clear();
            MidiBuffer midi;

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

        dummy.setCurrentProgram (0);
        expectEquals (dummy.getCurrentProgram(), presetMgr.getCurrentPresetIndex(), "Current program index incorrect!");

        expectEquals (dummy.getProgramName (0), presetMgr.getPresetName (0), "Program name incorrect!");

        auto userPresetConfigFile = presetMgr.getUserPresetConfigFile();
        userPresetConfigFile.deleteRecursively();
    }

    void runTest() override
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
    }
};

static PluginBaseTest pbTest;
