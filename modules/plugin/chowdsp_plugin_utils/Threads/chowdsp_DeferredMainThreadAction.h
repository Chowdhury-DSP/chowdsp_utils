#pragma once

#include <juce_dsp/juce_dsp.h>
#include "../../../dsp/chowdsp_dsp_data_structures/third_party/moodycamel/concurrentqueue.h"

namespace chowdsp
{
/**
 * Let's say you have a method that needs to run on the message thread,
 * but want to be able to trigger that action from any thread (even the
 * real-time thread). This class uses juce::AsyncUpdater, and a lock-free
 * queue to make that happen.
 */
class DeferredAction : private juce::Timer
{
public:
    /**
     * Constructs the deferred action handler with a given default
     * size to use for the lock-free queue.
     */
    explicit DeferredAction (int queueSize = 32) : queue ((size_t) queueSize)
    {
        startTimer (5);
    }

    /**
     * Calls the operation. If this is called from the main thread,
     * then the operation will happen immediately. Otherwise, the
     * operation will be deferred until the next async update.
     *
     * If this is being used in code that could be called from the audio
     * thread, make sure to set `couldBeAudioThread = true`.
     */
    template <typename Callable>
    void call (Callable&& operationToDefer, bool couldBeAudioThread = false)
    {
        if (juce::MessageManager::existsAndIsCurrentThread())
        {
            operationToDefer();
            return;
        }

        if (couldBeAudioThread)
        {
            const auto success = queue.try_enqueue (std::forward<Callable> (operationToDefer));

            // The queue doesn't have enough space for all these messages!
            // Consider changing the default size of the queue.
            jassert (success);
            juce::ignoreUnused (success);
        }
        else
        {
            // If we're sure this isn't the audio thread, we can safely allocate
            // memory if the queue needs to grow.
            queue.enqueue (std::forward<Callable> (operationToDefer));
        }

        callbacksReady.store (true);
    }

private:
    void timerCallback() override
    {
        if (AtomicHelpers::compareNegate (callbacksReady))
        {
            Action action;
            while (queue.try_dequeue (mainThreadConsumerToken, action))
                action();
        }
    }

    using Action = juce::dsp::FixedSizeFunction<256, void()>;
    moodycamel::ConcurrentQueue<Action> queue;

    moodycamel::ConsumerToken mainThreadConsumerToken { queue };

    std::atomic_bool callbacksReady { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeferredAction)
};

using SharedDeferredAction = juce::SharedResourcePointer<DeferredAction>;
} // namespace chowdsp
