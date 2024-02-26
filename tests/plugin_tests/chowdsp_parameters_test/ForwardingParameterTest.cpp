#include <CatchUtils.h>
#include <chowdsp_parameters/chowdsp_parameters.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct FPParams1 : chowdsp::ParamHolder
{
    FPParams1()
    {
        add (dummy);
    }
    chowdsp::PercentParameter::Ptr dummy { "dummy", "Dummy", 0.5f };
};

using State = chowdsp::PluginStateImpl<FPParams1>;

struct DummyPlugin : chowdsp::PluginBase<State>
{
    juce::UndoManager undoManager { 1000 };
    DummyPlugin() : chowdsp::PluginBase<chowdsp::PluginStateImpl<FPParams1>> (&undoManager)
    {
    }

    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>&) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
};

TEST_CASE ("Forwarding Parameter Test", "[plugin][parameters]")
{
    SECTION ("Null Parameter Test")
    {
        State state;
        std::unique_ptr<juce::AudioProcessorParameter> testParam = std::make_unique<chowdsp::ForwardingParameter> ("param", state, "NONE");

        testParam->setValue (0.5f);

        REQUIRE_MESSAGE (testParam->getName (1024) == juce::String ("NONE"), "Parameter name is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (testParam->getValue(), 0.0f), "Parameter value is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (testParam->getDefaultValue(), 0.0f), "Default parameter value is incorrect!");
        REQUIRE_MESSAGE (testParam->getText (0.2f, 1024) == juce::String(), "Parameter text is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (testParam->getValueForText ("0.9"), 0.0f), "Parameter value for text is incorrect!");

        auto& norm = dynamic_cast<juce::RangedAudioParameter*> (testParam.get())->getNormalisableRange();
        REQUIRE_MESSAGE (juce::approximatelyEqual (norm.interval, 0.01f), "Parameter normalization is incorrect!");

        auto* modulatableTestParam = dynamic_cast<chowdsp::ParamUtils::ModParameterMixin*> (testParam.get());
        REQUIRE_MESSAGE (! modulatableTestParam->supportsMonophonicModulation(), "Null parameter should not support modulation!");
        REQUIRE_MESSAGE (! modulatableTestParam->supportsPolyphonicModulation(), "Null parameter should not support modulation!");
    }

    SECTION ("Non-Null Parameter Test")
    {
        DummyPlugin dummy;
        auto* dummyParam = static_cast<juce::AudioProcessorParameter*> (dummy.getState().params.dummy.get());
        auto* forwardingParam = new chowdsp::ForwardingParameter ("param", dummy.getState(), "NONE");
        dummy.addParameter (forwardingParam);
        forwardingParam->setProcessor (&dummy);

        forwardingParam->setParam (dummy.getState().params.dummy.get());
        REQUIRE_MESSAGE (forwardingParam->getParam() == dummyParam, "Forwarding Parameter is not pointing to the correct parameter!");

        auto* testParam = (juce::AudioProcessorParameter*) forwardingParam;
        REQUIRE_MESSAGE (testParam->getName (1024) == dummyParam->getName (1024), "Parameter name is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (testParam->getDefaultValue(), dummyParam->getDefaultValue()), "Default parameter value is incorrect!");
        REQUIRE_MESSAGE (testParam->getText (0.2f, 1024) == dummyParam->getText (0.2f, 1024), "Parameter text is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (testParam->getValueForText ("0.9"), dummyParam->getValueForText ("0.9")), "Parameter value for text is incorrect!");

        auto& expNorm = dummy.getState().params.dummy->getNormalisableRange();
        auto& actualNorm = dynamic_cast<juce::RangedAudioParameter*> (testParam)->getNormalisableRange();
        REQUIRE_MESSAGE (juce::approximatelyEqual (actualNorm.start, expNorm.start), "Range start is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (actualNorm.end, expNorm.end), "Range end is incorrect!");

        REQUIRE_MESSAGE (juce::approximatelyEqual (testParam->getValue(), dummyParam->getValue()), "Initial parameter value is incorrect!");

        constexpr float error = 1.0e-6f;
        constexpr float value1 = 0.8f;
        testParam->beginChangeGesture();
        testParam->setValueNotifyingHost (value1);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        testParam->endChangeGesture();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        REQUIRE_MESSAGE (testParam->getValue() == Catch::Approx { value1 }.margin (error), "Forwarded param value set from forwarded param is incorrect!");
        REQUIRE_MESSAGE (dummyParam->getValue() == Catch::Approx { value1 }.margin (error), "Internal param value set from forwarded param is incorrect!");

        constexpr float value2 = 0.2f;
        dummy.undoManager.beginNewTransaction();
        dummy.undoManager.perform (
            new chowdsp::ParameterAttachmentHelpers::ParameterChangeAction<chowdsp::FloatParameter> { *dummy.getState().params.dummy,
                                                                                                      dummy.getState().params.dummy->get(),
                                                                                                      value2,
                                                                                                      dummy.getState().processor,
                                                                                                      false });
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        REQUIRE_MESSAGE (testParam->getValue() == Catch::Approx { value2 }.margin (error), "Forwarded param value set from internal param is incorrect!");
        REQUIRE_MESSAGE (dummyParam->getValue() == Catch::Approx { value2 }.margin (error), "Internal param value set from internal param is incorrect!");

        dummy.undoManager.undo();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        REQUIRE_MESSAGE (testParam->getValue() == Catch::Approx { value1 }.margin (error), "Forwarded param value set from undo is incorrect!");
        REQUIRE_MESSAGE (dummyParam->getValue() == Catch::Approx { value1 }.margin (error), "Internal param value set from undo is incorrect!");

        forwardingParam->setParam (nullptr);
        REQUIRE_MESSAGE (forwardingParam->getParam() == nullptr, "Forwarding Parameter should be pointing to anything!");

        dummyParam->beginChangeGesture();
        dummyParam->setValueNotifyingHost (value2);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        dummyParam->endChangeGesture();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        REQUIRE_MESSAGE (testParam->getValue() == Catch::Approx { 0.0f }.margin (error), "Disconnected forwarded param value is incorrect!");
        REQUIRE_MESSAGE (dummyParam->getValue() == Catch::Approx { value2 }.margin (error), "Disconnected internal param value is incorrect!");
    }

    SECTION ("Background Thread Test")
    {
        DummyPlugin dummy;
        auto* dummyParam = dummy.getState().params.dummy.get();

        auto* forwardingParam = new chowdsp::ForwardingParameter ("param", dummy.getState(), "NONE");
        dummy.addParameter (forwardingParam);
        forwardingParam->setProcessor (&dummy);
        forwardingParam->setParam (dummyParam, "Custom Name");

        auto* testParam = (juce::AudioProcessorParameter*) forwardingParam;
        REQUIRE_MESSAGE (testParam->getName (1024) == juce::String ("Custom Name"), "Custom parameter name is incorrect!");

        std::atomic<bool> threadFinished { false };
        juce::Thread::launch (
            [&]
            {
                constexpr float error = 1.0e-6f;
                constexpr float value1 = 0.8f;

                testParam->setValue (value1);
                testParam->sendValueChangedMessageToListeners (value1);

                juce::Thread::sleep (100);

                REQUIRE_MESSAGE (testParam->getValue() == Catch::Approx { value1 }.margin (error), "Forwarded param value set from forwarded param is incorrect!");
                REQUIRE_MESSAGE (static_cast<juce::AudioProcessorParameter*> (dummyParam)->getValue() == Catch::Approx { value1 }.margin (error), "Internal param value set from forwarded param is incorrect!");

                // do stuff on background thread
                threadFinished = true;
            });

        while (! threadFinished)
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
    }

    SECTION ("Parameter Modulation Test")
    {
        State state;
        auto&& testParam = std::make_unique<chowdsp::ForwardingParameter> ("param", state, "NONE");
        auto* testParamAsModParam = dynamic_cast<chowdsp::ParamUtils::ModParameterMixin*> (testParam.get());

        {
            static constexpr auto defaultValue = 0.5f;
            static constexpr auto modulationAmount = -0.25f;

            auto&& forwardParam = std::make_unique<juce::AudioParameterFloat> ("juce_float_param", "Param", 0.0f, 1.0f, defaultValue);
            testParam->setParam (forwardParam.get());

            REQUIRE_MESSAGE (! testParamAsModParam->supportsMonophonicModulation(), "juce::AudioParameterFloat should not support modulation!");
            REQUIRE_MESSAGE (! testParamAsModParam->supportsPolyphonicModulation(), "juce::AudioParameterFloat should not support modulation!");

            REQUIRE_MESSAGE (juce::approximatelyEqual (forwardParam->get(), defaultValue), "Parameter has incorrect value before parameter modulation!");
            testParamAsModParam->applyMonophonicModulation ((double) modulationAmount); // should have no effect, since the parameter doesn't support modulation!
            REQUIRE_MESSAGE (juce::approximatelyEqual (forwardParam->get(), defaultValue), "Parameter has incorrect value after parameter modulation!");

            testParam->setParam (nullptr);
        }

        {
            auto&& forwardParam = std::make_unique<chowdsp::ChoiceParameter> ("chowdsp_choice_param", "Param", juce::StringArray { "one", "two", "three" }, 0);
            testParam->setParam (forwardParam.get());

            REQUIRE_MESSAGE (! testParamAsModParam->supportsMonophonicModulation(), "chowdsp::ChoiceParameter should not support modulation!");
            REQUIRE_MESSAGE (! testParamAsModParam->supportsPolyphonicModulation(), "chowdsp::ChoiceParameter should not support modulation!");

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

            REQUIRE_MESSAGE (testParamAsModParam->supportsMonophonicModulation(), "chowdsp::FloatParameter should support monophonic modulation!");
            REQUIRE_MESSAGE (! testParamAsModParam->supportsPolyphonicModulation(), "chowdsp::FloatParameter should not support polyphonic modulation!");

            REQUIRE_MESSAGE (juce::approximatelyEqual (forwardParam->getCurrentValue(), defaultValue), "Parameter has incorrect value before parameter modulation!");
            testParamAsModParam->applyMonophonicModulation ((double) modulationAmount);
            REQUIRE_MESSAGE (juce::approximatelyEqual (forwardParam->getCurrentValue(), defaultValue + modulationAmount), "Parameter has incorrect value after parameter modulation!");

            testParam->setParam (nullptr);
        }
    }
}
