#include <TimedUnitTest.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

class StateListenersTest : public TimedUnitTest
{
public:
    StateListenersTest() : TimedUnitTest ("State Listeners Test", "ChowDSP State")
    {
    }

    void mainThreadListenersTest()
    {
        struct Params
        {
            chowdsp::PercentParameter::Ptr pct { "percent", "Percent", 1.0f };
        };

        chowdsp::PluginState<Params> state {};

        float mostRecentParamValue = -1.0f;
        int listenerCount = 0;
        chowdsp::ScopedCallback listener = state.addParameterListener (*state.params.pct,
                                                                       true,
                                                                       [this, &listenerCount, &state, &mostRecentParamValue]
                                                                       {
                                                                           expect (juce::MessageManager::getInstance()->isThisTheMessageThread(),
                                                                                   "Listener called on a thread other than the message thread!");
                                                                           expectEquals (state.params.pct->getCurrentValue(),
                                                                                         mostRecentParamValue,
                                                                                         "Parameter has the incorrect value when the listener is called!");
                                                                           listenerCount++;
                                                                       });

        static constexpr int numIters = 100;
        for (int i = 0; i < numIters; ++i)
        {
            mostRecentParamValue = (float) i / float (numIters - 1);
            static_cast<juce::AudioParameterFloat&> (state.params.pct) = mostRecentParamValue;
            juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
        }

        expectEquals (listenerCount, numIters, "Incorrect number of listener callbacks!");
    }

    void audioThreadListenersTest()
    {
        struct Params
        {
            chowdsp::PercentParameter::Ptr pct { "percent", "Percent", 1.0f };
        };

        chowdsp::PluginState<Params> state {};

        float mostRecentParamValue = -1.0f;
        int listenerCount = 0;
        chowdsp::ScopedCallback listener = state.addParameterListener (*state.params.pct,
                                                                       false,
                                                                       [this, &listenerCount, &state, &mostRecentParamValue]
                                                                       {
                                                                           expectEquals (state.params.pct->getCurrentValue(),
                                                                                         mostRecentParamValue,
                                                                                         "Parameter has the incorrect value when the listener is called!");
                                                                           listenerCount++;
                                                                       });

        static constexpr int numIters = 100;
        for (int i = 0; i < numIters; ++i)
        {
            mostRecentParamValue = (float) i / float (numIters - 1);
            static_cast<juce::AudioParameterFloat&> (state.params.pct) = mostRecentParamValue;
            juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
            state.callAudioThreadBroadcasters();
        }

        expectEquals (listenerCount, numIters, "Incorrect number of listener callbacks!");
    }

    void nonParameterListenersTest()
    {
        struct Params
        {
            chowdsp::PercentParameter::Ptr pct { "percent", "Percent", 1.0f };
        };

        struct NonParams
        {
            chowdsp::StateValue<int> value { "value", 100 };
        };

        static constexpr int newValue = 1000;
        bool listenerCalled = false;
        chowdsp::PluginState<Params, NonParams> state {};
        const auto listener = state.addNonParameterListener (state.nonParams.value,
                                                             [this, &state, &listenerCalled]
                                                             {
                                                                 listenerCalled = true;
                                                                 expectEquals ((int) state.nonParams.value, newValue, "Value after listerner callback is incorrect!");
                                                             });

        state.nonParams.value = newValue;

        expect (listenerCalled, "Listener was never called!");
    }

    void runTestTimed() override
    {
        beginTest ("Main Thread Listeners Test");
        mainThreadListenersTest();

        beginTest ("Audio Thread Listeners Test");
        audioThreadListenersTest();

        beginTest ("Non Parameter Listeners Test");
        nonParameterListenersTest();
    }
};

static StateListenersTest stateListenersTest;
