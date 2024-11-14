#pragma once

#if JUCE_MODULE_AVAILABLE_juce_dsp
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wimplicit-const-int-float-conversion")
#include <juce_dsp/juce_dsp.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#else
#include "../../../common/chowdsp_core/JUCEHelpers/juce_FixedSizeFunction.h"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#else
#if __has_include(<concurrentqueue.h>)
#include <concurrentqueue.h>
#else
#include "../../../dsp/chowdsp_dsp_data_structures/third_party/moodycamel/concurrentqueue.h"
#endif
#endif

namespace chowdsp
{
/**
 * Let's say you have a method that needs to run on the message thread,
 * but want to be able to trigger that action from any thread (even the
 * real-time thread). This class uses juce::AsyncUpdater, and a lock-free
 * queue to make that happen.
 */
class DeferredAction : public juce::Timer
{
public:
    /**
     * Constructs the deferred action handler with a given default
     * size to use for the lock-free queue.
     */
    explicit DeferredAction (int queueSize = 32) : queue ((size_t) queueSize)
    {
        startTimer (10);
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
    bool call (Callable&& operationToDefer, bool couldBeAudioThread = false)
    {
        if (juce::MessageManager::existsAndIsCurrentThread())
        {
            operationToDefer();
            return true;
        }

        auto success = true;
        if (couldBeAudioThread)
        {
            success = queue.try_enqueue (std::forward<Callable> (operationToDefer));

            // The queue doesn't have enough space for all these messages!
            // Consider changing the default size of the queue.
            jassert (success);
        }
        else
        {
            // If we're sure this isn't the audio thread, we can safely allocate
            // memory if the queue needs to grow.
            queue.enqueue (std::forward<Callable> (operationToDefer));
        }

        callbacksReady.store (true);
        return success;
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
