#include <TimedUnitTest.h>
#include <chowdsp_parameters/chowdsp_parameters.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

using namespace chowdsp::ParamUtils;

class ParamModulationTest : public TimedUnitTest
{
public:
    ParamModulationTest() : TimedUnitTest ("Parameter Modulation Test", "Parameters") {}

    void checkBoolParamModulation()
    {
        chowdsp::BoolParameter boolParam { "test", "Test", false };
        expect (! boolParam.supportsMonophonicModulation(), "Bool Parameters should not support modulation");
        expect (! boolParam.supportsPolyphonicModulation(), "Bool Parameters should not support modulation");
    }

    void checkChoiceParamModulation()
    {
        chowdsp::ChoiceParameter choiceParam { "test", "Test", juce::StringArray { "Choice 1", "Choice 1" }, 0 };
        expect (! choiceParam.supportsMonophonicModulation(), "Choice Parameters should not support modulation");
        expect (! choiceParam.supportsPolyphonicModulation(), "Choice Parameters should not support modulation");
    }

    void checkFloatParamModulation()
    {
        chowdsp::FloatParameter floatParam { "test", "Test", juce::NormalisableRange { 0.0f, 1.0f }, 0.5f, &floatValToString, &stringToFloatVal };
        expect (floatParam.supportsMonophonicModulation(), "Float Parameters should support monophonic modulation");
        expect (! floatParam.supportsPolyphonicModulation(), "Float Parameters should not support polyphonic modulation");
        expectEquals (floatParam.getDefaultValue(), 0.5f, "Float parameter default value is incorrect!");

        floatParam.applyMonophonicModulation (0.25);
        expectEquals ((float) floatParam, 0.75f, "Float parameter modulation is incorrect!");

        floatParam.setValueNotifyingHost (1.0f);
        floatParam.applyMonophonicModulation (-0.75);
        expectEquals (floatParam.getCurrentValue(), 0.25f, "Float parameter modulation is incorrect!");
    }

    void checkSmoothBufferedFloatParamModulation()
    {
        chowdsp::FloatParameter floatParam { "test", "Test", juce::NormalisableRange { 0.0f, 1.0f }, 0.5f, &floatValToString, &stringToFloatVal };

        static constexpr auto fs = 48000.0;
        static constexpr int blockSize = 2048;

        chowdsp::SmoothedBufferValue<float> smoothedParam;
        smoothedParam.setParameterHandle (&floatParam);
        smoothedParam.setRampLength ((double) blockSize / fs);
        smoothedParam.prepare (fs, blockSize);

        expectEquals (smoothedParam.getCurrentValue(), 0.5f, "Initial smoothed value is incorrect!");

        floatParam.applyMonophonicModulation (0.5);
        smoothedParam.process (blockSize);
        expectEquals (smoothedParam.getCurrentValue(), 1.0f, "Smoothed value after modulation is incorrect!");

        floatParam.setValueNotifyingHost (1.0f);
        floatParam.applyMonophonicModulation (-0.75);
        smoothedParam.process (blockSize);
        expectEquals (floatParam.getCurrentValue(), 0.25f, "Smoothed value after parameter change and modulation is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Check Bool Param Modulation");
        checkBoolParamModulation();

        beginTest ("Check Choice Param Modulation");
        checkChoiceParamModulation();

        beginTest ("Check Float Param Modulation");
        checkFloatParamModulation();

        beginTest ("Check Smooth Buffered Float Param Modulation");
        checkSmoothBufferedFloatParamModulation();
    }
};

static ParamModulationTest paramModulationTest;
