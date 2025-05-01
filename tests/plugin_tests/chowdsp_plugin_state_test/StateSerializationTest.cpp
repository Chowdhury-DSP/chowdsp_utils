#include <CatchUtils.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

struct LevelParams : chowdsp::ParamHolder
{
    explicit LevelParams (ParamHolder* parent, const juce::String& paramPrefix = "", int version = 100)
        : ParamHolder (parent),
          prefix (paramPrefix),
          versionHint (version)
    {
        add (percent, gain);
    }

    const juce::String prefix;
    const int versionHint;

    chowdsp::PercentParameter::Ptr percent { juce::ParameterID { prefix + "percent", versionHint }, "Percent" };
    chowdsp::GainDBParameter::Ptr gain { juce::ParameterID { prefix + "gain", versionHint }, "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
};

struct PluginParameterState : chowdsp::ParamHolder
{
    PluginParameterState()
    {
        add (levelParams, mode, onOff);
    }

    LevelParams levelParams { this };
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

struct PluginNonParameterState : chowdsp::NonParamState
{
    enum class YesNo
    {
        Yes,
        No
    };

    PluginNonParameterState()
    {
        addStateValues ({ &editorWidth, &editorHeight, &atomicThing, &jsonThing });

        for (size_t i = 0; i < 4; ++i)
        {
            yesNoNames[i] = chowdsp::StringLiteral { "yes_no" } + chowdsp::StringLiteral<1> { i };
            yesNoVals.emplace (i, yesNoNames[i], YesNo::No);
        }
        addStateValues (yesNoVals);
    }

    chowdsp::StateValue<int> editorWidth { "editor_width", 300 };
    chowdsp::StateValue<int> editorHeight { "editor_height", 500 };
    chowdsp::StateValue<std::atomic_int, int> atomicThing { "something_atomic", 12 };
    chowdsp::StateValue<nlohmann::json> jsonThing { "json_thing",
                                                    nlohmann::json { { "pi", 3.141 },
                                                                     { "happy", true },
                                                                     { "name", "Niels" },
                                                                     { "nothing", nullptr },
                                                                     { "answer", { { "everything", 42 } } },
                                                                     { "list", { 1, 0, 2 } },
                                                                     { "object", { { "currency", "USD" }, { "value", 42.99 } } } } };

    std::array<chowdsp::StringLiteral<8>, 8> yesNoNames {};
    chowdsp::OptionalArray<chowdsp::StateValue<YesNo>, 8> yesNoVals;
};

using State = chowdsp::PluginStateImpl<PluginParameterState, PluginNonParameterState>;

struct PluginParameterStateNewParam : chowdsp::ParamHolder
{
    PluginParameterStateNewParam()
    {
        add (newParam2, onOff, mode, newParam3, levelParams, newParam);
    }

    LevelParams levelParams { this };
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
    chowdsp::GainDBParameter::Ptr newParam { "gain_new", "New Gain", juce::NormalisableRange { -45.0f, 12.0f }, 3.3f };
    chowdsp::ChoiceParameter::Ptr newParam2 { "choice_new", "New Choice", juce::StringArray { "a", "b", "c" }, 2 };
    chowdsp::BoolParameter::Ptr newParam3 { "bool_new", "New Bool", false };
};

using StateWithNewParam = chowdsp::PluginStateImpl<PluginParameterStateNewParam>;

struct PluginParameterStateRemovedParam : chowdsp::ParamHolder
{
    PluginParameterStateRemovedParam()
    {
        add (levelParams, onOff);
    }

    LevelParams levelParams { this };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

using StateWithRemovedParam = chowdsp::PluginStateImpl<PluginParameterStateRemovedParam>;

struct PluginParameterStateDoubleOfSameType : chowdsp::ParamHolder
{
    PluginParameterStateDoubleOfSameType()
    {
        add (levelParams1, levelParams2, mode, onOff);
    }

    LevelParams levelParams1 { this, "level_params1" };
    LevelParams levelParams2 { this, "level_params2", 101 };
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

using StateWithDoubleOfSameType = chowdsp::PluginStateImpl<PluginParameterStateDoubleOfSameType>;

struct PluginParameterStateTripleOfSameType : chowdsp::ParamHolder
{
    PluginParameterStateTripleOfSameType()
    {
        add (levelParams1, levelParams2, levelParams3, mode, onOff);
    }

    LevelParams levelParams1 { this, "level_params1" };
    LevelParams levelParams2 { this, "level_params2", 101 };
    LevelParams levelParams3 { this, "level_params3", 102 };
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

using StateWithTripleOfSameType = chowdsp::PluginStateImpl<PluginParameterStateTripleOfSameType>;

struct NewGroup : chowdsp::ParamHolder
{
    explicit NewGroup (ParamHolder* parent)
        : ParamHolder { parent }
    {
        add (newParam);
    }

    static constexpr std::string_view name { "new_group" };
    chowdsp::GainDBParameter::Ptr newParam { "gain_new", "New Gain", juce::NormalisableRange { -45.0f, 12.0f }, 3.3f };
};

struct PluginParameterStateNewGroup : chowdsp::ParamHolder
{
    PluginParameterStateNewGroup()
    {
        add (levelParams, mode, onOff, newGroup);
    }

    LevelParams levelParams { this };
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
    NewGroup newGroup { this };
};

using StateWithNewGroup = chowdsp::PluginStateImpl<PluginParameterStateNewGroup>;

struct PluginNonParameterStateNewField : chowdsp::NonParamState
{
    PluginNonParameterStateNewField()
    {
        addStateValues ({ &editorWidth, &editorHeight, &randomString });
    }

    chowdsp::StateValue<int> editorWidth { "editor_width", 300 };
    chowdsp::StateValue<int> editorHeight { "editor_height", 500 };
    chowdsp::StateValue<juce::String> randomString { "random_string", "default" };
};

using StateWithNewNonParameterField = chowdsp::PluginStateImpl<PluginParameterState, PluginNonParameterStateNewField>;

TEST_CASE ("State Serialization Test", "[plugin][state][serial]")
{
    SECTION ("Save/Load Parameters Test")
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
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams.percent->get(), percentVal), "Percent value is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams.gain->get(), gainVal), "Gain value is incorrect");
        REQUIRE_MESSAGE (state.params.mode->getIndex() == choiceVal, "Choice value is incorrect");
        REQUIRE_MESSAGE (state.params.onOff->get() == boolVal, "Bool value is incorrect");
        REQUIRE_MESSAGE (! um.canUndo(), "Undo manager was not cleared after loading new state!");
    }

    SECTION ("Save/Load Non-Parameters Test")
    {
        static constexpr int width = 200;
        static constexpr int height = 150;
        static constexpr int atomic = 24;
        const auto testJSON = nlohmann::json { { "new", 20 } };

        juce::MemoryBlock block;
        {
            State state;
            state.nonParams.editorWidth = width;
            state.nonParams.editorHeight = height;
            state.nonParams.atomicThing = atomic;
            state.nonParams.jsonThing = testJSON;
            state.serialize (block);
        }

        State state;
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (state.nonParams.editorWidth.get() == width, "Editor width is incorrect");
        REQUIRE_MESSAGE (state.nonParams.editorHeight.get() == height, "Editor height is incorrect");
        REQUIRE_MESSAGE (state.nonParams.atomicThing.get() == atomic, "Atomic thing is incorrect");
        REQUIRE_MESSAGE (state.nonParams.jsonThing.get() == testJSON, "JSON thing is incorrect");
    }

    SECTION ("Load invalid state")
    {
        static constexpr float percentVal = 0.25f;
        static constexpr int width = 200;

        State state {};
        static_cast<juce::AudioParameterFloat&> (state.params.levelParams.percent) = percentVal;
        state.nonParams.editorWidth = width;

        juce::MemoryBlock block {};
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams.percent->get(), 0.5f), "Percent value is incorrect");
        REQUIRE_MESSAGE (state.nonParams.editorWidth.get() == 300, "Editor width is incorrect");
    }

    SECTION ("Added Parameter Test")
    {
        static constexpr float newGainVal = -22.0f;
        static constexpr int newChoiceVal = 0;
        static constexpr bool newBoolVal = true;

        juce::MemoryBlock block;
        {
            State state;
            state.serialize (block);
        }

        StateWithNewParam state;
        static_cast<juce::AudioParameterFloat&> (state.params.newParam) = newGainVal;
        static_cast<juce::AudioParameterChoice&> (state.params.newParam2) = newChoiceVal;
        static_cast<juce::AudioParameterBool&> (state.params.newParam3) = newBoolVal;
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (state.params.newParam->get() == Catch::Approx (3.3f).margin (1.0e-6f), "Added param value is incorrect");
        REQUIRE_MESSAGE (state.params.newParam2->getIndex() == 2, "Added param value is incorrect");
        REQUIRE_MESSAGE (state.params.newParam3->get() == false, "Added param value is incorrect");
    }

    SECTION ("Removed Parameter Test")
    {
        static constexpr float percentVal = 0.25f;
        static constexpr float gainVal = -22.0f;
        static constexpr bool boolVal = false;

        juce::MemoryBlock block;
        {
            State state;
            static_cast<juce::AudioParameterFloat&> (state.params.levelParams.percent) = percentVal;
            static_cast<juce::AudioParameterFloat&> (state.params.levelParams.gain) = gainVal;
            static_cast<juce::AudioParameterBool&> (state.params.onOff) = boolVal;
            state.serialize (block);
        }

        StateWithRemovedParam state {};
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams.percent->get(), percentVal), "Percent value is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams.gain->get(), gainVal), "Gain value is incorrect");
        REQUIRE_MESSAGE (state.params.onOff->get() == boolVal, "Bool value is incorrect");
    }

    SECTION ("Added Parameter Group Test")
    {
        static constexpr float newGainVal = -22.0f;

        juce::MemoryBlock block;
        {
            State state;
            state.serialize (block);
        }

        StateWithNewGroup state;
        static_cast<juce::AudioParameterFloat&> (state.params.newGroup.newParam) = newGainVal;
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (state.params.newGroup.newParam->get() == Catch::Approx (3.3f).margin (1.0e-6f), "Added param value is incorrect");
    }

    SECTION ("Double of Same Type Test")
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
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams1.percent->get(), percentVal1), "Percent value 1 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams1.gain->get(), gainVal1), "Gain value 1 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams2.percent->get(), percentVal2), "Percent value 2 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams2.gain->get(), gainVal2), "Gain value 2 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams3.percent->get(), 0.5f), "Percent value 3 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams3.gain->get(), 0.0f), "Gain value 3 is incorrect");
        REQUIRE_MESSAGE (state.params.mode->getIndex() == choiceVal, "Choice value is incorrect");
        REQUIRE_MESSAGE (state.params.onOff->get() == boolVal, "Bool value is incorrect");
    }

    SECTION ("Added Non-Parameter Field Test")
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
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (state.nonParams.editorWidth.get() == width, "Editor width is incorrect");
        REQUIRE_MESSAGE (state.nonParams.editorHeight.get() == height, "Editor height is incorrect");
        REQUIRE_MESSAGE (state.nonParams.randomString.get() == juce::String { "default" }, "Added field is incorrect");
    }
}

TEST_CASE ("Legacy State Serialization Test", "[plugin][state]")
{
    SECTION ("Save/Load Parameters Test")
    {
        static constexpr float percentVal = 0.25f;
        static constexpr float gainVal = -22.0f;
        static constexpr int choiceVal = 0;
        static constexpr bool boolVal = false;

        const auto jsonState = nlohmann::json::parse (R"(["9.9.9",["editor_width",300,"editor_height",500,"something_atomic",12,"json_thing",{"answer":{"everything":42},"happy":true,"list":[1,0,2],"name":"Niels","nothing":null,"object":{"currency":"USD","value":42.99},"pi":3.141},"yes_no0",1,"yes_no1",1,"yes_no2",1,"yes_no3",1],["percent",0.25,"gain",-22.0,"mode",0,"on_off",false]])");
        juce::MemoryBlock block;
        chowdsp::JSONUtils::toMemoryBlock (jsonState, block);

        struct DummyAction : juce::UndoableAction
        {
            bool perform() override { return true; }
            bool undo() override { return true; }
        };

        juce::UndoManager um { 100 };
        State state { &um };
        um.beginNewTransaction();
        um.perform (new DummyAction {});
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams.percent->get(), percentVal), "Percent value is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams.gain->get(), gainVal), "Gain value is incorrect");
        REQUIRE_MESSAGE (state.params.mode->getIndex() == choiceVal, "Choice value is incorrect");
        REQUIRE_MESSAGE (state.params.onOff->get() == boolVal, "Bool value is incorrect");
        REQUIRE_MESSAGE (! um.canUndo(), "Undo manager was not cleared after loading new state!");
    }

    SECTION ("Save/Load Non-Parameters Test")
    {
        static constexpr int width = 200;
        static constexpr int height = 150;
        static constexpr int atomic = 24;
        const auto testJSON = nlohmann::json { { "new", 20 } };

        const auto jsonState = nlohmann::json::parse (R"(["9.9.9",["editor_width",200,"editor_height",150,"something_atomic",24,"json_thing",{"new":20},"yes_no0",1,"yes_no1",1,"yes_no2",1,"yes_no3",1],["percent",0.5,"gain",0.0,"mode",2,"on_off",true]])");
        juce::MemoryBlock block;
        chowdsp::JSONUtils::toMemoryBlock (jsonState, block);

        State state;
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (state.nonParams.editorWidth.get() == width, "Editor width is incorrect");
        REQUIRE_MESSAGE (state.nonParams.editorHeight.get() == height, "Editor height is incorrect");
        REQUIRE_MESSAGE (state.nonParams.atomicThing.get() == atomic, "Atomic thing is incorrect");
        REQUIRE_MESSAGE (state.nonParams.jsonThing.get() == testJSON, "JSON thing is incorrect");
    }

    SECTION ("Added Parameter Test")
    {
        static constexpr float newGainVal = -22.0f;

        const auto jsonState = nlohmann::json::parse (R"(["9.9.9",["editor_width",300,"editor_height",500,"something_atomic",12,"json_thing",{"answer":{"everything":42},"happy":true,"list":[1,0,2],"name":"Niels","nothing":null,"object":{"currency":"USD","value":42.99},"pi":3.141},"yes_no0",1,"yes_no1",1,"yes_no2",1,"yes_no3",1],["percent",0.5,"gain",0.0,"mode",2,"on_off",true]])");
        juce::MemoryBlock block;
        chowdsp::JSONUtils::toMemoryBlock (jsonState, block);

        StateWithNewParam state;
        static_cast<juce::AudioParameterFloat&> (state.params.newParam) = newGainVal;
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (state.params.newParam->get() == Catch::Approx (3.3f).margin (1.0e-6f), "Added param value is incorrect");
    }

    SECTION ("Added Parameter Group Test")
    {
        static constexpr float newGainVal = -22.0f;

        const auto jsonState = nlohmann::json::parse (R"(["9.9.9",["editor_width",300,"editor_height",500,"something_atomic",12,"json_thing",{"answer":{"everything":42},"happy":true,"list":[1,0,2],"name":"Niels","nothing":null,"object":{"currency":"USD","value":42.99},"pi":3.141},"yes_no0",1,"yes_no1",1,"yes_no2",1,"yes_no3",1],["percent",0.5,"gain",0.0,"mode",2,"on_off",true]])");
        juce::MemoryBlock block;
        chowdsp::JSONUtils::toMemoryBlock (jsonState, block);

        StateWithNewGroup state;
        static_cast<juce::AudioParameterFloat&> (state.params.newGroup.newParam) = newGainVal;
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (state.params.newGroup.newParam->get() == Catch::Approx (3.3f).margin (1.0e-6f), "Added param value is incorrect");
    }

    SECTION ("Double of Same Type Test")
    {
        static constexpr float percentVal1 = 0.25f;
        static constexpr float gainVal1 = -22.0f;
        static constexpr float percentVal2 = 0.85f;
        static constexpr float gainVal2 = -29.0f;
        static constexpr int choiceVal = 0;
        static constexpr bool boolVal = false;

        const auto jsonState = nlohmann::json::parse (R"(["9.9.9",[],["level_params1percent",0.25,"level_params1gain",-22.0,"level_params2percent",0.8500000238418579,"level_params2gain",-29.0,"mode",0,"on_off",false]])");
        juce::MemoryBlock block;
        chowdsp::JSONUtils::toMemoryBlock (jsonState, block);

        StateWithTripleOfSameType state {};
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams1.percent->get(), percentVal1), "Percent value 1 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams1.gain->get(), gainVal1), "Gain value 1 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams2.percent->get(), percentVal2), "Percent value 2 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams2.gain->get(), gainVal2), "Gain value 2 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams3.percent->get(), 0.5f), "Percent value 3 is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (state.params.levelParams3.gain->get(), 0.0f), "Gain value 3 is incorrect");
        REQUIRE_MESSAGE (state.params.mode->getIndex() == choiceVal, "Choice value is incorrect");
        REQUIRE_MESSAGE (state.params.onOff->get() == boolVal, "Bool value is incorrect");
    }

    SECTION ("Added Non-Parameter Field Test")
    {
        static constexpr int width = 200;
        static constexpr int height = 150;

        const auto jsonState = nlohmann::json::parse (R"(["9.9.9",["editor_width",200,"editor_height",150,"something_atomic",12,"json_thing",{"answer":{"everything":42},"happy":true,"list":[1,0,2],"name":"Niels","nothing":null,"object":{"currency":"USD","value":42.99},"pi":3.141},"yes_no0",1,"yes_no1",1,"yes_no2",1,"yes_no3",1],["percent",0.5,"gain",0.0,"mode",2,"on_off",true]])");
        juce::MemoryBlock block;
        chowdsp::JSONUtils::toMemoryBlock (jsonState, block);

        StateWithNewNonParameterField state;
        state.deserialize (std::move (block));
        REQUIRE_MESSAGE (state.nonParams.editorWidth.get() == width, "Editor width is incorrect");
        REQUIRE_MESSAGE (state.nonParams.editorHeight.get() == height, "Editor height is incorrect");
        REQUIRE_MESSAGE (state.nonParams.randomString.get() == juce::String { "default" }, "Added field is incorrect");
    }
}
