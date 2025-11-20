#include <CatchUtils.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

TEST_CASE ("Bool Parameter Test", "[plugin][parameters]")
{
    SECTION ("Custom labels")
    {
        juce::String offLabel { "bar" };
        juce::String onLabel { "foo" };

        auto&& param = chowdsp::BoolParameter ("bool", "Bool", false, offLabel, onLabel);
        REQUIRE (param.getCurrentValueAsText() == offLabel);

        param.setParameterValue (true);
        REQUIRE (param.getCurrentValueAsText() == onLabel);

        auto* p = dynamic_cast<juce::AudioProcessorParameter*> (&param);
        REQUIRE (p != nullptr);

        for (auto str : { onLabel, onLabel.toUpperCase(), juce::String ("1") })
            REQUIRE_MESSAGE (p->getValueForText (str) == 1.0f, str + " should mean true");

        for (auto str : { offLabel, offLabel.toUpperCase(), juce::String ("0") })
            REQUIRE_MESSAGE (p->getValueForText (str) == 0.0f, str + " should mean false");
    }
}