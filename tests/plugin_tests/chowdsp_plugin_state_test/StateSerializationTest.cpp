#include <TimedUnitTest.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

struct LevelParams
{
    const std::string_view name = "level_params";
    const int version = 100;
    chowdsp::PercentParameter::Ptr percent { juce::ParameterID { "percent", version }, "Percent" };
    chowdsp::GainDBParameter::Ptr gain { juce::ParameterID { "gain", version }, "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
};

struct PluginParameterState
{
    LevelParams levelParams;
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

struct PluginNonParameterState
{
    chowdsp::StateValue<int> editorWidth { "editor_width", 300 };
    chowdsp::StateValue<int> editorHeight { "editor_height", 500 };
};

using State = chowdsp::PluginState<PluginParameterState, PluginNonParameterState>;

struct PluginParameterStateNewParam
{
    LevelParams levelParams;
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
    chowdsp::GainDBParameter::Ptr newParam { "gain_new", "New Gain", juce::NormalisableRange { -45.0f, 12.0f }, 3.3f };
};

using StateWithNewParam = chowdsp::PluginState<PluginParameterStateNewParam>;

struct PluginParameterStateDoubleOfSameType
{
    LevelParams levelParams1 { "level_params1" };
    LevelParams levelParams2 { "level_params2", 101 };
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

using StateWithDoubleOfSameType = chowdsp::PluginState<PluginParameterStateDoubleOfSameType>;

struct PluginParameterStateTripleOfSameType
{
    LevelParams levelParams1 { "level_params1" };
    LevelParams levelParams2 { "level_params2", 101 };
    LevelParams levelParams3 { "level_params3", 102 };
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

using StateWithTripleOfSameType = chowdsp::PluginState<PluginParameterStateTripleOfSameType>;

struct NewGroup
{
    static constexpr std::string_view name { "new_group" };
    chowdsp::GainDBParameter::Ptr newParam { "gain_new", "New Gain", juce::NormalisableRange { -45.0f, 12.0f }, 3.3f };
};

struct PluginParameterStateNewGroup
{
    LevelParams levelParams;
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
    NewGroup newGroup;
};

using StateWithNewGroup = chowdsp::PluginState<PluginParameterStateNewGroup>;

struct PluginNonParameterStateNewField
{
    chowdsp::StateValue<int> editorWidth { "editor_width", 300 };
    chowdsp::StateValue<int> editorHeight { "editor_height", 500 };
    chowdsp::StateValue<juce::String> randomString { "random_string", "default" };
};

using StateWithNewNonParameterField = chowdsp::PluginState<PluginParameterState, PluginNonParameterStateNewField>;

class StateSerializationTest : public TimedUnitTest
{
public:
    StateSerializationTest() : TimedUnitTest ("State Serialization Test", "ChowDSP State")
    {
    }

    void saveLoadParametersTest()
    {
        static constexpr float percentVal = 0.25f;
        static constexpr float gainVal = -22.0f;
        static constexpr int choiceVal = 0;
        static constexpr bool boolVal = false;

        juce::MemoryBlock block;
        {
            State state;
            static_cast<juce::AudioParameterFloat&> (state.params.levelParams.percent) = percentVal;
            static_cast<juce::AudioParameterFloat&> (state.params.levelParams.gain) = gainVal;
            static_cast<juce::AudioParameterChoice&> (state.params.mode) = choiceVal;
            static_cast<juce::AudioParameterBool&> (state.params.onOff) = boolVal;
            state.serialize (block);
        }

        struct DummyAction : juce::UndoableAction
        {
            bool perform() override { return true; }
            bool undo() override { return true; }
        };

        juce::UndoManager um { 100 };
        State state { &um };
        um.beginNewTransaction();
        um.perform (new DummyAction {});
        state.deserialize (block);
        expectEquals (state.params.levelParams.percent->get(), percentVal, "Percent value is incorrect");
        expectEquals (state.params.levelParams.gain->get(), gainVal, "Gain value is incorrect");
        expectEquals (state.params.mode->getIndex(), choiceVal, "Choice value is incorrect");
        expect (state.params.onOff->get() == boolVal, "Bool value is incorrect");
        expect (! um.canUndo(), "Undo manager was not cleared after loading new state!");
    }

    void saveLoadNonParametersTest()
    {
        static constexpr int width = 200;
        static constexpr int height = 150;

        juce::MemoryBlock block;
        {
            State state;
            state.nonParams.editorWidth = width;
            state.nonParams.editorHeight = height;
            state.serialize (block);
        }

        State state;
        state.deserialize (block);
        expectEquals (state.nonParams.editorWidth.get(), width, "Editor width is incorrect");
        expectEquals (state.nonParams.editorHeight.get(), height, "Editor height is incorrect");
    }

    void addedParamTest()
    {
        static constexpr float newGainVal = -22.0f;

        juce::MemoryBlock block;
        {
            State state;
            state.serialize (block);
        }

        StateWithNewParam state;
        static_cast<juce::AudioParameterFloat&> (state.params.newParam) = newGainVal;
        state.deserialize (block);
        expectWithinAbsoluteError (state.params.newParam->get(), 3.3f, 1.0e-6f, "Added param value is incorrect");
    }

    void addedGroupTest()
    {
        static constexpr float newGainVal = -22.0f;

        juce::MemoryBlock block;
        {
            State state;
            state.serialize (block);
        }

        StateWithNewGroup state;
        static_cast<juce::AudioParameterFloat&> (state.params.newGroup.newParam) = newGainVal;
        state.deserialize (block);
        expectWithinAbsoluteError (state.params.newGroup.newParam->get(), 3.3f, 1.0e-6f, "Added param value is incorrect");
    }

    void multipleOfSameTypeTest()
    {
        static constexpr float percentVal1 = 0.25f;
        static constexpr float gainVal1 = -22.0f;
        static constexpr float percentVal2 = 0.85f;
        static constexpr float gainVal2 = -29.0f;
        static constexpr int choiceVal = 0;
        static constexpr bool boolVal = false;

        juce::MemoryBlock block;
        {
            StateWithDoubleOfSameType state;
            static_cast<juce::AudioParameterFloat&> (state.params.levelParams1.percent) = percentVal1;
            static_cast<juce::AudioParameterFloat&> (state.params.levelParams1.gain) = gainVal1;
            static_cast<juce::AudioParameterFloat&> (state.params.levelParams2.percent) = percentVal2;
            static_cast<juce::AudioParameterFloat&> (state.params.levelParams2.gain) = gainVal2;
            static_cast<juce::AudioParameterChoice&> (state.params.mode) = choiceVal;
            static_cast<juce::AudioParameterBool&> (state.params.onOff) = boolVal;
            state.serialize (block);
        }

        StateWithTripleOfSameType state {};
        state.deserialize (block);
        expectEquals (state.params.levelParams1.percent->get(), percentVal1, "Percent value 1 is incorrect");
        expectEquals (state.params.levelParams1.gain->get(), gainVal1, "Gain value 1 is incorrect");
        expectEquals (state.params.levelParams2.percent->get(), percentVal2, "Percent value 2 is incorrect");
        expectEquals (state.params.levelParams2.gain->get(), gainVal2, "Gain value 2 is incorrect");
        expectEquals (state.params.levelParams3.percent->get(), 0.5f, "Percent value 3 is incorrect");
        expectEquals (state.params.levelParams3.gain->get(), 0.0f, "Gain value 3 is incorrect");
        expectEquals (state.params.mode->getIndex(), choiceVal, "Choice value is incorrect");
        expect (state.params.onOff->get() == boolVal, "Bool value is incorrect");
    }

    void addedNonParameterFieldTest()
    {
        static constexpr int width = 200;
        static constexpr int height = 150;

        juce::MemoryBlock block;
        {
            State state;
            state.nonParams.editorWidth = width;
            state.nonParams.editorHeight = height;
            state.serialize (block);
        }

        StateWithNewNonParameterField state;
        state.deserialize (block);
        expectEquals (state.nonParams.editorWidth.get(), width, "Editor width is incorrect");
        expectEquals (state.nonParams.editorHeight.get(), height, "Editor height is incorrect");
        expectEquals (state.nonParams.randomString.get(), juce::String { "default" }, "Added field is incorrect");
    }

    void runTestTimed() override
    {
        beginTest ("Save/Load Parameters Test");
        saveLoadParametersTest();

        beginTest ("Save/Load Non-Parameters Test");
        saveLoadNonParametersTest();

        beginTest ("Added Parameter Test");
        addedParamTest();

        beginTest ("Added Parameter Group Test");
        addedGroupTest();

        beginTest ("Double of Same Type Test");
        multipleOfSameTypeTest();

        beginTest ("Added Non-Parameter Field Test");
        addedNonParameterFieldTest();
    }
};

static StateSerializationTest stateSerializationTest;
