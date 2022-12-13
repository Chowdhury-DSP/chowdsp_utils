#include <DummyPlugin.h>
#include <TimedUnitTest.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

class ForwardingParameterTest : public TimedUnitTest
{
public:
    ForwardingParameterTest() : TimedUnitTest ("Forwarding Parameter Test", "Parameters")
    {
    }

    void nullParamTest()
    {
        std::unique_ptr<juce::AudioProcessorParameter> testParam = std::make_unique<chowdsp::ForwardingParameter> ("param", nullptr, "NONE");

        testParam->setValue (0.5f);

        expectEquals (testParam->getName (1024), juce::String ("NONE"), "Parameter name is incorrect!");
        expectEquals (testParam->getValue(), 0.0f, "Parameter value is incorrect!");
        expectEquals (testParam->getDefaultValue(), 0.0f, "Default parameter value is incorrect!");
        expectEquals (testParam->getText (0.2f, 1024), juce::String(), "Parameter text is incorrect!");
        expectEquals (testParam->getValueForText ("0.9"), 0.0f, "Parameter value for text is incorrect!");

        auto& norm = dynamic_cast<juce::RangedAudioParameter*> (testParam.get())->getNormalisableRange();
        expectEquals (norm.interval, 0.01f, "Parameter normalization is incorrect!");

        auto* modulatableTestParam = dynamic_cast<chowdsp::ParamUtils::ModParameterMixin*> (testParam.get());
        expect (! modulatableTestParam->supportsMonophonicModulation(), "Null parameter should not support modulation!");
        expect (! modulatableTestParam->supportsPolyphonicModulation(), "Null parameter should not support modulation!");
    }

    void nonNullParamTest()
    {
        DummyPlugin dummy;
        auto* dummyParam = dummy.getVTS().getParameter ("dummy");

        auto& undoManager = dummy.getUndoManager();
        auto* forwardingParam = new chowdsp::ForwardingParameter ("param", &undoManager, "NONE");
        dummy.addParameter (forwardingParam);
        forwardingParam->setProcessor (&dummy);

        forwardingParam->setParam (dummyParam);
        expect (forwardingParam->getParam() == dummyParam, "Forwarding Parameter is not pointing to the correct parameter!");

        auto* testParam = (juce::AudioProcessorParameter*) forwardingParam;
        expectEquals (testParam->getName (1024), dummyParam->getName (1024), "Parameter name is incorrect!");
        expectEquals (testParam->getDefaultValue(), dummyParam->getDefaultValue(), "Default parameter value is incorrect!");
        expectEquals (testParam->getText (0.2f, 1024), dummyParam->getText (0.2f, 1024), "Parameter text is incorrect!");
        expectEquals (testParam->getValueForText ("0.9"), dummyParam->getValueForText ("0.9"), "Parameter value for text is incorrect!");

        auto& expNorm = dummyParam->getNormalisableRange();
        auto& actualNorm = dynamic_cast<juce::RangedAudioParameter*> (testParam)->getNormalisableRange();
        expectEquals (actualNorm.start, expNorm.start, "Range start is incorrect!");
        expectEquals (actualNorm.end, expNorm.end, "Range end is incorrect!");

        expectEquals (testParam->getValue(), dummyParam->getValue(), "Initial parameter value is incorrect!");

        constexpr float error = 1.0e-6f;
        constexpr float value1 = 0.8f;
        testParam->beginChangeGesture();
        testParam->setValueNotifyingHost (value1);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        testParam->endChangeGesture();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        expectWithinAbsoluteError (testParam->getValue(), value1, error, "Forwarded param value set from forwarded param is incorrect!");
        expectWithinAbsoluteError (dummyParam->getValue(), value1, error, "Internal param value set from forwarded param is incorrect!");

        constexpr float value2 = 0.2f;
        dummyParam->beginChangeGesture();
        dummyParam->setValueNotifyingHost (value2);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        dummyParam->endChangeGesture();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        expectWithinAbsoluteError (testParam->getValue(), value2, error, "Forwarded param value set from internal param is incorrect!");
        expectWithinAbsoluteError (dummyParam->getValue(), value2, error, "Internal param value set from internal param is incorrect!");

        undoManager.undo();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        expectWithinAbsoluteError (testParam->getValue(), value1, error, "Forwarded param value set from undo is incorrect!");
        expectWithinAbsoluteError (dummyParam->getValue(), value1, error, "Internal param value set from undo is incorrect!");

        forwardingParam->setParam (nullptr);
        expect (forwardingParam->getParam() == nullptr, "Forwarding Parameter should be pointing to anything!");

        dummyParam->beginChangeGesture();
        dummyParam->setValueNotifyingHost (value2);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        dummyParam->endChangeGesture();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        expectWithinAbsoluteError (testParam->getValue(), 0.0f, error, "Disconnected forwarded param value is incorrect!");
        expectWithinAbsoluteError (dummyParam->getValue(), value2, error, "Disconnected internal param value is incorrect!");
    }

    void backgroundThreadTest()
    {
        DummyPlugin dummy;
        auto* dummyParam = dummy.getVTS().getParameter ("dummy");

        auto& undoManager = dummy.getUndoManager();
        auto* forwardingParam = new chowdsp::ForwardingParameter ("param", &undoManager, "NONE");
        dummy.addParameter (forwardingParam);
        forwardingParam->setProcessor (&dummy);
        forwardingParam->setParam (dummyParam, "Custom Name");

        auto* testParam = (juce::AudioProcessorParameter*) forwardingParam;
        expectEquals (testParam->getName (1024), juce::String ("Custom Name"), "Custom parameter name is incorrect!");

        std::atomic<bool> threadFinished { false };
        juce::Thread::launch ([&]
                              {
            constexpr float error = 1.0e-6f;
            constexpr float value1 = 0.8f;

            testParam->setValue (value1);
            testParam->sendValueChangedMessageToListeners (value1);

            juce::Thread::sleep (100);

            expectWithinAbsoluteError (testParam->getValue(), value1, error, "Forwarded param value set from forwarded param is incorrect!");
            expectWithinAbsoluteError (dummyParam->getValue(), value1, error, "Internal param value set from forwarded param is incorrect!");

            // do stuff on background thread
            threadFinished = true; });

        while (! threadFinished)
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
    }

    void parameterModulationTest()
    {
        auto&& testParam = std::make_unique<chowdsp::ForwardingParameter> ("param", nullptr, "NONE");
        auto* testParamAsModParam = dynamic_cast<chowdsp::ParamUtils::ModParameterMixin*> (testParam.get());

        {
            static constexpr auto defaultValue = 0.5f;
            static constexpr auto modulationAmount = -0.25f;

            auto&& forwardParam = std::make_unique<juce::AudioParameterFloat> ("juce_float_param", "Param", 0.0f, 1.0f, defaultValue);
            testParam->setParam (forwardParam.get());

            expect (! testParamAsModParam->supportsMonophonicModulation(), "juce::AudioParameterFloat should not support modulation!");
            expect (! testParamAsModParam->supportsPolyphonicModulation(), "juce::AudioParameterFloat should not support modulation!");

            expectEquals (forwardParam->get(), defaultValue, "Parameter has incorrect value before parameter modulation!");
            testParamAsModParam->applyMonophonicModulation ((double) modulationAmount); // should have no effect, since the parameter doesn't support modulation!
            expectEquals (forwardParam->get(), defaultValue, "Parameter has incorrect value after parameter modulation!");

            testParam->setParam (nullptr);
        }

        {
            auto&& forwardParam = std::make_unique<chowdsp::ChoiceParameter> ("chowdsp_choice_param", "Param", juce::StringArray { "one", "two", "three" }, 0);
            testParam->setParam (forwardParam.get());

            expect (! testParamAsModParam->supportsMonophonicModulation(), "chowdsp::ChoiceParameter should not support modulation!");
            expect (! testParamAsModParam->supportsPolyphonicModulation(), "chowdsp::ChoiceParameter should not support modulation!");

            testParam->setParam (nullptr);
        }

        {
            static constexpr auto defaultValue = 0.5f;
            static constexpr auto modulationAmount = -0.25f;
            auto&& forwardParam = std::make_unique<chowdsp::FloatParameter> ("chowdsp_float_param",
                                                                             "Param",
                                                                             juce::NormalisableRange { 0.0f, 1.0f },
                                                                             defaultValue,
                                                                             &chowdsp::ParamUtils::floatValToString,
                                                                             &chowdsp::ParamUtils::stringToFloatVal);
            testParam->setParam (forwardParam.get());

            expect (testParamAsModParam->supportsMonophonicModulation(), "chowdsp::FloatParameter should support monophonic modulation!");
            expect (! testParamAsModParam->supportsPolyphonicModulation(), "chowdsp::FloatParameter should not support polyphonic modulation!");

            expectEquals (forwardParam->getCurrentValue(), defaultValue, "Parameter has incorrect value before parameter modulation!");
            testParamAsModParam->applyMonophonicModulation ((double) modulationAmount);
            expectEquals (forwardParam->getCurrentValue(), defaultValue + modulationAmount, "Parameter has incorrect value after parameter modulation!");

            testParam->setParam (nullptr);
        }
    }

    void runTestTimed() override
    {
        beginTest ("Null Parameter Test");
        nullParamTest();

        beginTest ("Non-Null Parameter Test");
        nonNullParamTest();

        beginTest ("Background Thread Test");
        backgroundThreadTest();

        beginTest ("Parameter Modulation Test");
        parameterModulationTest();
    }
};

static ForwardingParameterTest forwardingParameterTest;
