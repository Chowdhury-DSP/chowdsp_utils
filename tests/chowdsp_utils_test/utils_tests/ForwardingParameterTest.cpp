#include <DummyPlugin.h>

class ForwardingParameterTest : public UnitTest
{
public:
    ForwardingParameterTest() : UnitTest ("Forwarding Parameter Test")
    {
    }

    void nullParamTest()
    {
        std::unique_ptr<AudioProcessorParameter> testParam = std::make_unique<chowdsp::ForwardingParameter> ("param", "NONE");

        testParam->setValue (0.5f);

        expectEquals (testParam->getName (1024), String ("NONE"), "Parameter name is incorrect!");
        expectEquals (testParam->getValue(), 0.0f, "Parameter value is incorrect!");
        expectEquals (testParam->getDefaultValue(), 0.0f, "Default parameter value is incorrect!");
        expectEquals (testParam->getText (0.2f, 1024), String(), "Parameter text is incorrect!");
        expectEquals (testParam->getValueForText ("0.9"), 0.0f, "Parameter value for text is incorrect!");

        auto& norm = dynamic_cast<RangedAudioParameter*> (testParam.get())->getNormalisableRange();
        expectEquals (norm.interval, 0.01f, "Parameter normalization is incorrect!");
    }

    void nonNullParamTest()
    {
        DummyPlugin dummy;
        auto* dummyParam = dummy.getVTS().getParameter ("dummy");

        auto forwardingParam = std::make_unique<chowdsp::ForwardingParameter> ("param", "NONE");
        forwardingParam->setProcessor (&dummy);
        forwardingParam->setParam (dummyParam);

        auto* testParam = (AudioProcessorParameter*) forwardingParam.get();
        expectEquals (testParam->getName (1024), dummyParam->getName (1024), "Parameter name is incorrect!");
        expectEquals (testParam->getDefaultValue(), dummyParam->getDefaultValue(), "Default parameter value is incorrect!");
        expectEquals (testParam->getText (0.2f, 1024), dummyParam->getText (0.2f, 1024), "Parameter text is incorrect!");
        expectEquals (testParam->getValueForText ("0.9"), dummyParam->getValueForText ("0.9"), "Parameter value for text is incorrect!");

        auto& expNorm = dummyParam->getNormalisableRange();
        auto& actualNorm = dynamic_cast<RangedAudioParameter*> (testParam)->getNormalisableRange();
        expectEquals (actualNorm.start, expNorm.start, "Range start is incorrect!");
        expectEquals (actualNorm.end, expNorm.end, "Range end is incorrect!");

        expectEquals (testParam->getValue(), dummyParam->getValue(), "Initial parameter value is incorrect!");
        testParam->setValue (0.5f);
        expectEquals (testParam->getValue(), dummyParam->getValue(), "Set parameter value is incorrect!");
    }

    void runTest() override
    {
        beginTest ("Null Parameter Test");
        nullParamTest();

        beginTest ("Non-Null Parameter Test");
        nonNullParamTest();
    }
};

static ForwardingParameterTest forwardingParameterTest;
