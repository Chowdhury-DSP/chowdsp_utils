#include <CatchUtils.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

TEST_CASE ("State Listeners Test", "[state][listeners]")
{
    SECTION ("Main Thread Listeners Test")
    {
        chowdsp::ParamHolder params {};
        chowdsp::PercentParameter::Ptr pct { "percent", "Percent", 1.0f };
        params.add (pct);
        chowdsp::ParameterListeners listeners { params };

        float mostRecentParamValue = -1.0f;
        int listenerCount = 0;
        chowdsp::ScopedCallback listener = listeners.addParameterListener (
            pct,
            chowdsp::ParameterListenerThread::MessageThread,
            [&listenerCount, &pct, &mostRecentParamValue]
            {
                REQUIRE_MESSAGE (juce::MessageManager::getInstance()->isThisTheMessageThread(),
                                 "Listener called on a thread other than the message thread!");
                REQUIRE_MESSAGE (pct->getCurrentValue() == mostRecentParamValue,
                                 "Parameter has the incorrect value when the listener is called!");
                listenerCount++;
            });

        static constexpr int numIters = 50;
        for (int i = 0; i < numIters; ++i)
        {
            mostRecentParamValue = (float) i / float (numIters - 1);
            static_cast<juce::AudioParameterFloat&> (pct) = mostRecentParamValue;
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
        }

        REQUIRE_MESSAGE (listenerCount == Catch::Approx (numIters).margin (2), "Incorrect number of listener callbacks!");
    }

    SECTION ("Audio Thread Listeners Test")
    {
        chowdsp::ParamHolder params {};
        chowdsp::PercentParameter::Ptr pct { "percent", "Percent", 1.0f };
        params.add (pct);
        chowdsp::ParameterListeners listeners { params };

        float mostRecentParamValue = -1.0f;
        int listenerCount = 0;
        chowdsp::ScopedCallback listener = listeners.addParameterListener (
            pct,
            chowdsp::ParameterListenerThread::AudioThread,
            [&listenerCount, &pct, &mostRecentParamValue]
            {
                REQUIRE_MESSAGE (pct->getCurrentValue() == mostRecentParamValue,
                                 "Parameter has the incorrect value when the listener is called!");
                listenerCount++;
            });

        static constexpr int numIters = 50;
        for (int i = 0; i < numIters; ++i)
        {
            mostRecentParamValue = (float) i / float (numIters - 1);
            static_cast<juce::AudioParameterFloat&> (pct) = mostRecentParamValue;
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
            listeners.callAudioThreadBroadcasters();
        }

        REQUIRE_MESSAGE (listenerCount == Catch::Approx (numIters).margin (2), "Incorrect number of listener callbacks!");
    }

    SECTION ("Non Parameter Listeners Test")
    {
        struct Params : chowdsp::ParamHolder
        {
            Params()
            {
                add (pct);
            }

            chowdsp::PercentParameter::Ptr pct { "percent", "Percent", 1.0f };
        };

        struct NonParams : chowdsp::NonParamState
        {
            NonParams() : chowdsp::NonParamState ({ &value }) {}
            chowdsp::StateValue<int> value { "value", 100 };
        };

        static constexpr int newValue = 1000;
        bool listenerCalled = false;
        chowdsp::PluginStateImpl<Params, NonParams> state {};
        const auto listener = state.addNonParameterListener (
            state.nonParams.value,
            [&state, &listenerCalled]
            {
                listenerCalled = true;
                REQUIRE_MESSAGE ((int) state.nonParams.value == newValue, "Value after listener callback is incorrect!");
            });

        state.nonParams.value = newValue;

        REQUIRE_MESSAGE (listenerCalled, "Listener was never called!");
    }
}
