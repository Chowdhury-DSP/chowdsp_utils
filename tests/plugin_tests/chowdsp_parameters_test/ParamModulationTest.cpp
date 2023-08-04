#include <CatchUtils.h>
#include <chowdsp_parameters/chowdsp_parameters.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

using namespace chowdsp::ParamUtils;

TEST_CASE ("Parameter Modulation Test", "[plugin][parameters]")
{
    SECTION ("Check Bool Param Modulation")
    {
        chowdsp::BoolParameter boolParam { "test", "Test", false };
        REQUIRE_MESSAGE (! boolParam.supportsMonophonicModulation(), "Bool Parameters should not support modulation");
        REQUIRE_MESSAGE (! boolParam.supportsPolyphonicModulation(), "Bool Parameters should not support modulation");
    }

    SECTION ("Check Choice Param Modulation")
    {
        chowdsp::ChoiceParameter choiceParam { "test", "Test", juce::StringArray { "Choice 1", "Choice 1" }, 0 };
        REQUIRE_MESSAGE (! choiceParam.supportsMonophonicModulation(), "Choice Parameters should not support modulation");
        REQUIRE_MESSAGE (! choiceParam.supportsPolyphonicModulation(), "Choice Parameters should not support modulation");
    }

    SECTION ("Check Float Param Modulation")
    {
        chowdsp::FloatParameter floatParam { "test", "Test", juce::NormalisableRange { 0.0f, 1.0f }, 0.5f, &floatValToString, &stringToFloatVal };
        REQUIRE_MESSAGE (floatParam.supportsMonophonicModulation(), "Float Parameters should support monophonic modulation");
        REQUIRE_MESSAGE (! floatParam.supportsPolyphonicModulation(), "Float Parameters should not support polyphonic modulation");
        REQUIRE_MESSAGE (juce::approximatelyEqual (floatParam.getDefaultValue(), 0.5f), "Float parameter default value is incorrect!");

        floatParam.applyMonophonicModulation (0.25);
        REQUIRE_MESSAGE (juce::approximatelyEqual ((float) floatParam, 0.75f), "Float parameter modulation is incorrect!");

        floatParam.setValueNotifyingHost (1.0f);
        floatParam.applyMonophonicModulation (-0.75);
        REQUIRE_MESSAGE (juce::approximatelyEqual (floatParam.getCurrentValue(), 0.25f), "Float parameter modulation is incorrect!");
    }

    SECTION ("Check Smooth Buffered Float Param Modulation")
    {
        chowdsp::FloatParameter floatParam { "test", "Test", juce::NormalisableRange { 0.0f, 1.0f }, 0.5f, &floatValToString, &stringToFloatVal };

        static constexpr auto fs = 48000.0;
        static constexpr int blockSize = 2048;

        chowdsp::SmoothedBufferValue<float> smoothedParam;
        smoothedParam.setParameterHandle (&floatParam);
        smoothedParam.setRampLength ((double) blockSize / fs);
        smoothedParam.prepare (fs, blockSize);

        REQUIRE_MESSAGE (juce::approximatelyEqual (smoothedParam.getCurrentValue(), 0.5f), "Initial smoothed value is incorrect!");

        floatParam.applyMonophonicModulation (0.5);
        smoothedParam.process (blockSize);
        REQUIRE_MESSAGE (juce::approximatelyEqual (smoothedParam.getCurrentValue(), 1.0f), "Smoothed value after modulation is incorrect!");

        floatParam.setValueNotifyingHost (1.0f);
        floatParam.applyMonophonicModulation (-0.75);
        smoothedParam.process (blockSize);
        REQUIRE_MESSAGE (juce::approximatelyEqual (floatParam.getCurrentValue(), 0.25f), "Smoothed value after parameter change and modulation is incorrect!");
    }
}
