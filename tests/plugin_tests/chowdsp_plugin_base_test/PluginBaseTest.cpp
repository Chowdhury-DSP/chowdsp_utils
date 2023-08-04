#include <CatchUtils.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

TEST_CASE ("Plugin Base Test", "[plugin]")
{
    struct Params : chowdsp::ParamHolder
    {
        Params()
        {
            add (param);
        }
        chowdsp::PercentParameter::Ptr param { "param", "Param", 0.5f };
    };

    struct DummyPlugin : chowdsp::PluginBase<chowdsp::PluginStateImpl<Params>>
    {
        void releaseResources() override {}
        void processAudioBlock (juce::AudioBuffer<float>& buffer) override { buffer.clear(); }
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    };

    SECTION ("Save/Load State")
    {
        static constexpr float testValue = 0.234f;

        DummyPlugin dummy1;
        dummy1.getParameters()[0]->setValue (testValue);

        juce::MemoryBlock stateBlock;
        dummy1.getStateInformation (stateBlock);

        DummyPlugin dummy2;
        dummy2.setStateInformation (stateBlock.getData(), (int) stateBlock.getSize());

        juce::MemoryBlock stateBlock2;
        dummy2.getStateInformation (stateBlock2);

        REQUIRE_MESSAGE (stateBlock == stateBlock2, "Plugin base did not load correct state!");
    }

    SECTION ("Buses Layout Test")
    {
        DummyPlugin dummy;

        juce::AudioProcessor::BusesLayout stLayout { { juce::AudioChannelSet::stereo() }, { juce::AudioChannelSet::stereo() } };
        REQUIRE_MESSAGE (dummy.isBusesLayoutSupported (stLayout), "Stereo I/O should be supported!");

        juce::AudioProcessor::BusesLayout monoLayout { { juce::AudioChannelSet::mono() }, { juce::AudioChannelSet::mono() } };
        REQUIRE_MESSAGE (dummy.isBusesLayoutSupported (monoLayout), "Mono I/O should be supported!");

        juce::AudioProcessor::BusesLayout smLayout { { juce::AudioChannelSet::stereo() }, { juce::AudioChannelSet::mono() } };
        REQUIRE_MESSAGE (! dummy.isBusesLayoutSupported (smLayout), "Stereo in, mono out should NOT be supported!");

        juce::AudioProcessor::BusesLayout msLayout { { juce::AudioChannelSet::mono() }, { juce::AudioChannelSet::stereo() } };
        REQUIRE_MESSAGE (! dummy.isBusesLayoutSupported (msLayout), "Mono in, stereo out should NOT be supported!");

        juce::AudioProcessor::BusesLayout quadLayout { { juce::AudioChannelSet::quadraphonic() }, { juce::AudioChannelSet::quadraphonic() } };
        REQUIRE_MESSAGE (! dummy.isBusesLayoutSupported (quadLayout), "More than two channels should NOT be supported!");
    }

    SECTION ("Properties Test")
    {
        DummyPlugin dummy1;

        REQUIRE_MESSAGE (dummy1.getName() == juce::String ("TestPlugin"), "Plugin name incorrect!");

        bool midiBehaviour = dummy1.acceptsMidi() || dummy1.producesMidi() || dummy1.isMidiEffect();
        REQUIRE_MESSAGE (! midiBehaviour, "MIDI behaviour incorrect!");

        REQUIRE_MESSAGE (juce::exactlyEqual (dummy1.getTailLengthSeconds(), 0.0), "Tail length incorrect!");

        REQUIRE_MESSAGE (dummy1.getNumPrograms() > 0, "Num programs is to low!");
        dummy1.setCurrentProgram (0);
        REQUIRE_MESSAGE (dummy1.getCurrentProgram() == 0, "Current program is incorrect!");
        dummy1.changeProgramName (0, "");
        REQUIRE_MESSAGE (dummy1.getProgramName (0) == juce::String(), "Program name incorrect");
    }

    //        SECTION ("GUI Test")
    //        {
    //            DummyPlugin dummy1;
    //
    //            if (dummy1.hasEditor())
    //            {
    //                std::unique_ptr<juce::AudioProcessorEditor> editor (dummy1.createEditor());
    //
    //                expect (editor != nullptr, "Editor should not be nullptr!");
    //
    //                if (editor != nullptr)
    //                {
    //                    editor->setSize (700, 700); // make editor larger
    //                    dummy1.editorBeingDeleted (editor.get());
    //                }
    //            }
    //            else
    //            {
    //                expect (false, "Dummy plugin has no editor!");
    //            }
    //        }

    SECTION ("Process Test")
    {
        DummyPlugin dummy1;
        constexpr int blockSize = 1024;
        dummy1.prepareToPlay (48000.0, blockSize);

        juce::AudioBuffer<float> buffer (2, blockSize);
        for (auto [_, channel] : chowdsp::buffer_iters::channels (buffer))
            std::fill (channel.begin(), channel.end(), 1.0f);
        juce::MidiBuffer midi;

        dummy1.processBlock (buffer, midi);
        REQUIRE_MESSAGE (buffer.getMagnitude (0, blockSize) < 1.0e-3f, "Buffer should be empty!");
    }
}
