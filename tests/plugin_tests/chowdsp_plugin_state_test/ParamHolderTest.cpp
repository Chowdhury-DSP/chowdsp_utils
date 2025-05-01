#include <CatchUtils.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

TEST_CASE ("ParamHolder Test", "[plugin][state][serial]")
{
    std::array<chowdsp::PercentParameter::Ptr, 2> floatParams {
        chowdsp::PercentParameter::Ptr { "param3", "Param", 0.5f },
        chowdsp::PercentParameter::Ptr { "param4", "Param", 0.5f },
    };
    chowdsp::BoolParameter::Ptr boolNested { "param1", "Param", false };
    chowdsp::ChoiceParameter::Ptr choiceNested { "param2", "Param", juce::StringArray { "One", "Two" }, 0 };

    chowdsp::ParamHolder params;
    chowdsp::ParamHolder nestedParams { &params };
    nestedParams.add (boolNested, choiceNested);
    params.add (nestedParams, floatParams);

    SECTION ("add()")
    {
        auto allParamIDs = [&params]
        {
            juce::StringArray allIDs;
            params.doForAllParameters ([&allIDs] (auto& param, size_t)
                                       { allIDs.add (param.paramID); });
            return allIDs;
        }();

        REQUIRE (allParamIDs.contains (boolNested->paramID));
        REQUIRE (allParamIDs.contains (choiceNested->paramID));
        REQUIRE (allParamIDs.contains (floatParams[0]->paramID));
        REQUIRE (allParamIDs.contains (floatParams[1]->paramID));
    }

    SECTION ("Serialize JSON")
    {
        using namespace chowdsp::ParameterTypeHelpers;
        setValue (0.0f, *floatParams[0]);
        setValue (1.0f, *floatParams[1]);
        setValue (true, *boolNested);
        setValue (1, *choiceNested);

        const auto json_state = chowdsp::ParamHolder::serialize_json (params);
        params.doForAllParameters ([] (auto& param, size_t)
                                   { setValue (getDefaultValue (param), param); });

        REQUIRE (getValue (*floatParams[0]) == 0.5f);
        REQUIRE (getValue (*floatParams[1]) == 0.5f);
        REQUIRE (getValue (*boolNested) == false);
        REQUIRE (getValue (*choiceNested) == 0);

        chowdsp::ParamHolder::deserialize_json (json_state, params);
        REQUIRE (getValue (*floatParams[0]) == 0.0f);
        REQUIRE (getValue (*floatParams[1]) == 1.0f);
        REQUIRE (getValue (*boolNested) == true);
        REQUIRE (getValue (*choiceNested) == 1);
    }

    SECTION ("reset()")
    {
        using namespace chowdsp::ParameterTypeHelpers;
        setValue (0.0f, *floatParams[0]);
        setValue (1.0f, *floatParams[1]);
        setValue (true, *boolNested);
        setValue (1, *choiceNested);

        params.reset();
        REQUIRE (getValue (*floatParams[0]) == 0.5f);
        REQUIRE (getValue (*floatParams[1]) == 0.5f);
        REQUIRE (getValue (*boolNested) == false);
        REQUIRE (getValue (*choiceNested) == 0);
    }
}
