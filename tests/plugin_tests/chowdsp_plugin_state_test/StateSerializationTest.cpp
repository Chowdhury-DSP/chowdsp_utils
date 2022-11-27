#include <TimedUnitTest.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

struct LevelParams
{
    static constexpr std::string_view name = "level_params";
    chowdsp::PercentParameter::Ptr percent { "percent", "Percent" };
    chowdsp::GainDBParameter::Ptr gain { "gain", "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
};

struct PluginParameterState
{
    LevelParams levelParams;
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

struct PluginNonParameterState
{
    int editorWidth = 300;
    int editorHeight = 500;
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

        State state;
        state.deserialize (block);
        expectEquals (state.params.levelParams.percent->get(), percentVal, "Percent value is incorrect");
        expectEquals (state.params.levelParams.gain->get(), gainVal, "Gain value is incorrect");
        expectEquals (state.params.mode->getIndex(), choiceVal, "Choice value is incorrect");
        expect (state.params.onOff->get() == boolVal, "Bool value is incorrect");
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
        expectEquals (state.nonParams.editorWidth, width, "Editor width is incorrect");
        expectEquals (state.nonParams.editorHeight, height, "Editor height is incorrect");
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

    void runTestTimed() override
    {
        beginTest ("Save/Load Parameters Test");
        saveLoadParametersTest();

        beginTest ("Save/Load Non-Parameters Test");

        beginTest ("Added Parameter Test");
        addedParamTest();

        beginTest ("Added Parameter Group Test");
        addedGroupTest();
    }
};

static StateSerializationTest stateSerializationTest;
