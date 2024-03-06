#include <CatchUtils.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

TEST_CASE ("ParamHolder Test", "[plugin][state]")
{
    SECTION ("add()")
    {
        chowdsp::BoolParameter::Ptr boolNested { "param1", "Param", false };
        chowdsp::ChoiceParameter::Ptr choiceNested { "param2", "Param", juce::StringArray { "One", "Two" }, 0 };
        chowdsp::ParamHolder nestedParams;
        nestedParams.add (boolNested, choiceNested);

        chowdsp::ParamHolder params;
        std::array<chowdsp::PercentParameter::Ptr, 2> floatParams {
            chowdsp::PercentParameter::Ptr { "param3", "Param", 0.5f },
            chowdsp::PercentParameter::Ptr { "param4", "Param", 0.5f },
        };
        params.add (nestedParams, floatParams);

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

    SECTION ("clear()")
    {
        chowdsp::ParamHolder params { "Params", false };
        chowdsp::PercentParameter::Ptr pct_param { "percent", "Percent", 0.5f };
        chowdsp::PercentParameter::Ptr pct_param2 { "percent2", "Percent 2", 0.5f };
        params.add (pct_param, pct_param2);
        REQUIRE (params.count() == 2);

        params.clear();
        REQUIRE (params.count() == 0);
    }
}
