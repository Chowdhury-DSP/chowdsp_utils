#include <DummyPlugin.h>
#include <TimedUnitTest.h>

class ForwardingParameterTest : public TimedUnitTest
{
public:
    ForwardingParameterTest() : TimedUnitTest ("Forwarding Parameter Test")
    {
    }

    void nullParamTest()
    {
        std::unique_ptr<AudioProcessorParameter> testParam = std::make_unique<chowdsp::ForwardingParameter> ("param", nullptr, "NONE");

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

        auto& undoManager = dummy.getUndoManager();
        auto* forwardingParam = new chowdsp::ForwardingParameter ("param", &undoManager, "NONE");
        dummy.addParameter (forwardingParam);
        forwardingParam->setProcessor (&dummy);
        forwardingParam->setParam (dummyParam);

        auto* testParam = (AudioProcessorParameter*) forwardingParam;
        expectEquals (testParam->getName (1024), dummyParam->getName (1024), "Parameter name is incorrect!");
        expectEquals (testParam->getDefaultValue(), dummyParam->getDefaultValue(), "Default parameter value is incorrect!");
        expectEquals (testParam->getText (0.2f, 1024), dummyParam->getText (0.2f, 1024), "Parameter text is incorrect!");
        expectEquals (testParam->getValueForText ("0.9"), dummyParam->getValueForText ("0.9"), "Parameter value for text is incorrect!");

        auto& expNorm = dummyParam->getNormalisableRange();
        auto& actualNorm = dynamic_cast<RangedAudioParameter*> (testParam)->getNormalisableRange();
        expectEquals (actualNorm.start, expNorm.start, "Range start is incorrect!");
        expectEquals (actualNorm.end, expNorm.end, "Range end is incorrect!");

        expectEquals (testParam->getValue(), dummyParam->getValue(), "Initial parameter value is incorrect!");

        constexpr float error = 1.0e-6f;
        constexpr float value1 = 0.8f;
        testParam->beginChangeGesture();
        testParam->setValueNotifyingHost (value1);
        MessageManager::getInstance()->runDispatchLoopUntil (100);
        testParam->endChangeGesture();
        MessageManager::getInstance()->runDispatchLoopUntil (100);
        expectWithinAbsoluteError (testParam->getValue(), value1, error, "Forwarded param value set from forwarded param is incorrect!");
        expectWithinAbsoluteError (dummyParam->getValue(), value1, error, "Internal param value set from forwarded param is incorrect!");

        constexpr float value2 = 0.2f;
        dummyParam->beginChangeGesture();
        dummyParam->setValueNotifyingHost (value2);
        MessageManager::getInstance()->runDispatchLoopUntil (100);
        dummyParam->endChangeGesture();
        MessageManager::getInstance()->runDispatchLoopUntil (100);
        expectWithinAbsoluteError (testParam->getValue(), value2, error, "Forwarded param value set from internal param is incorrect!");
        expectWithinAbsoluteError (dummyParam->getValue(), value2, error, "Internal param value set from internal param is incorrect!");

        undoManager.undo();
        MessageManager::getInstance()->runDispatchLoopUntil (100);
        expectWithinAbsoluteError (testParam->getValue(), value1, error, "Forwarded param value set from undo is incorrect!");
        expectWithinAbsoluteError (dummyParam->getValue(), value1, error, "Internal param value set from undo is incorrect!");

        forwardingParam->setParam (nullptr);
        dummyParam->beginChangeGesture();
        dummyParam->setValueNotifyingHost (value2);
        MessageManager::getInstance()->runDispatchLoopUntil (100);
        dummyParam->endChangeGesture();
        MessageManager::getInstance()->runDispatchLoopUntil (100);
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

        auto* testParam = (AudioProcessorParameter*) forwardingParam;
        expectEquals (testParam->getName (1024), String ("Custom Name"), "Custom parameter name is incorrect!");

        std::atomic<bool> threadFinished { false };
        Thread::launch ([&]
                        {
                            constexpr float error = 1.0e-6f;
                            constexpr float value1 = 0.8f;

                            testParam->setValue (value1);
                            testParam->sendValueChangedMessageToListeners (value1);

                            Thread::sleep (100);

                            expectWithinAbsoluteError (testParam->getValue(), value1, error, "Forwarded param value set from forwarded param is incorrect!");
                            expectWithinAbsoluteError (dummyParam->getValue(), value1, error, "Internal param value set from forwarded param is incorrect!");

                            // do stuff on background thread
                            threadFinished = true;
                        });

        while (! threadFinished)
            MessageManager::getInstance()->runDispatchLoopUntil (100);
    }

    void runTestTimed() override
    {
        beginTest ("Null Parameter Test");
        nullParamTest();

        beginTest ("Non-Null Parameter Test");
        nonNullParamTest();

        beginTest ("Background Thread Test");
        backgroundThreadTest();
    }
};

static ForwardingParameterTest forwardingParameterTest;
