#include <CatchUtils.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

TEST_CASE ("Metric Parameter Test", "[plugin][parameters]")
{
    SECTION ("Resistance Parameter")
    {
        chowdsp::MetricParameter param {
            "param",
            "Param",
            juce::NormalisableRange { 1.0e-18f, 1.0e18f },
            1.0f,
            juce::String::fromUTF8 ("Ω"),
            1
        };

        const auto getValueForText = [&param]
        {
            const auto text = param.getCurrentValueAsText();
            const auto normValue = static_cast<juce::AudioProcessorParameter&> (param).getValueForText (text);
            return param.convertFrom0to1 (normValue);
        };

        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("1.0 Ω"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-6));

        param.setParameterValue (1.5e3f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("1.5 kΩ"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-6));

        param.setParameterValue (2.5e6f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("2.5 MΩ"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-6));

        param.setParameterValue (2.5e6f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("2.5 MΩ"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-6));

        param.setParameterValue (2.5e9f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("2.5 GΩ"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-6));
    }

    SECTION ("Capacitance Parameter")
    {
        chowdsp::MetricParameter param {
            "param",
            "Param",
            juce::NormalisableRange { 1.0e-18f, 1.0e18f },
            1.0f,
            juce::String::fromUTF8 ("F"),
            2
        };

        const auto getValueForText = [&param]
        {
            const auto text = param.getCurrentValueAsText();
            const auto normValue = static_cast<juce::AudioProcessorParameter&> (param).getValueForText (text);
            return param.convertFrom0to1 (normValue);
        };

        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("1.00 F"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-6));

        param.setParameterValue (1.5e-3f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("1.50 mF"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-18));

        param.setParameterValue (2.55e-6f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("2.55 μF"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-21));

        param.setParameterValue (7.7e-9f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("7.70 nF"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-24));

        param.setParameterValue (7.7e-12f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("7.70 pF"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-27));
    }
}
