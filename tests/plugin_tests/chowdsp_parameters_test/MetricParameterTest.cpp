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

        param.setParameterValue (7.7e-15f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("7.70 fF"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-30));
    }

    SECTION ("Voltage Parameter")
    {
        chowdsp::MetricParameter param {
            "param",
            "Param",
            juce::NormalisableRange { -2.0f, 2.0f },
            0.0f,
            juce::String::fromUTF8 ("V"),
            2
        };

        const auto getValueForText = [&param]
        {
            const auto text = param.getCurrentValueAsText();
            const auto normValue = static_cast<juce::AudioProcessorParameter&> (param).getValueForText (text);
            return param.convertFrom0to1 (normValue);
        };

        param.setParameterValue (1.0f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("1.00 V"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-6));

        param.setParameterValue (-1.0f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("-1.00 V"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-6));

        param.setParameterValue (-0.001f);
        REQUIRE (param.getCurrentValueAsText() == juce::String::fromUTF8 ("-1.00 mV"));
        REQUIRE (getValueForText() == Catch::Approx { param.get() }.margin (1.0e-6));
    }

    SECTION ("Decimal Places")
    {
        static constexpr auto test_val = 1.54321f;
        static constexpr auto test_val_milli = 1.54321e-3f;
        std::array<std::tuple<int, std::string_view, std::string_view>, 5> tests {
            std::tuple<int, std::string_view, std::string_view> { 4, "1.5432 V", "1.5432 mV" },
            { 3, "1.543 V", "1.543 mV" },
            { 2, "1.54 V", "1.54 mV" },
            { 1, "1.5 V", "1.5 mV" },
            { 0, "1 V", "1 mV" },
        };

        for (auto [num_decimals, test_str, test_str_milli] : tests)
        {
            chowdsp::MetricParameter param {
                "param",
                "Param",
                juce::NormalisableRange { -2.0f, 2.0f },
                0.0f,
                juce::String::fromUTF8 ("V"),
                num_decimals,
            };

            param.setParameterValue (test_val);
            REQUIRE (param.getCurrentValueAsText() == chowdsp::toString (test_str));

            if (num_decimals < 4)
            {
                param.setParameterValue (test_val_milli);
                REQUIRE (param.getCurrentValueAsText() == chowdsp::toString (test_str_milli));
            }
        }
    }
}
