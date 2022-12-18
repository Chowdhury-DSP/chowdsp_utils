#pragma once

#if JUCE_MODULE_AVAILABLE_juce_dsp
#include <juce_dsp/juce_dsp.h>
#else
#include "../../../common/chowdsp_core/JUCEHelpers/juce_FixedSizeFunction.h"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#else
#include "../../../dsp/chowdsp_dsp_data_structures/third_party/moodycamel/readerwriterqueue.h"
#endif

namespace chowdsp
{
/** Enum to specify which thread a parameter listener callback should happen on. */
enum class ParameterListenerThread
{
    MessageThread,
    AudioThread,
};

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324) // struct was padded warning

/** Utility class to manage a set of parameter listeners. */
class ParameterListeners : private juce::AsyncUpdater
{
public:
    /**
     * Initialises the listeners with a set of parameters.
     *
     * By default, this class will create it's own background thread to use for triggering
     * the listeners, but users can also supply their own thread to use instead.
     */
    explicit ParameterListeners (ParamHolder& parameters, juce::TimeSliceThread* backgroundThread = nullptr, int backgroundThreadIntervalMilliseconds = 10);

    /** Destructor */
    ~ParameterListeners() override;

    /**
     * Runs any queued listeners on the audio thread.
     *
     * If you're using chowdsp::PluginBase, this will get called automatically,
     * otherwise, users must call this in the audio callback themselves!
     */
    void callAudioThreadBroadcasters();

    /**
     * Runs any queued listeners on the message thread.
     *
     * This will be called automatically whenever a parameter changes (via juce::AsyncUpdater),
     * but users can also call this on the message thread manually if they like.
     */
    void callMessageThreadBroadcasters();

    /** Creates a new parameter listener. */
    template <typename... ListenerArgs>
    [[nodiscard]] ScopedCallback addParameterListener (const juce::RangedAudioParameter& param, ParameterListenerThread listenerThread, ListenerArgs&&... args)
    {
        const auto paramInfoIter = std::find_if (paramInfoList.begin(), paramInfoList.end(), [&param] (const ParamInfo& info)
                                                 { return info.paramCookie == &param; });

        if (paramInfoIter == paramInfoList.end())
        {
            jassertfalse; // trying to listen to a parameter that is not part of this state!
            return {};
        }

        const auto index = (size_t) std::distance (paramInfoList.begin(), paramInfoIter);
        auto& broadcasterList = listenerThread == ParameterListenerThread::MessageThread ? messageThreadBroadcasters : audioThreadBroadcasters;
        return broadcasterList[index].connect (std::forward<ListenerArgs...> (args...));
    }

private:
    void callMessageThreadBroadcaster (size_t index);
    void callAudioThreadBroadcaster (size_t index);
    void handleAsyncUpdate() override;

    OptionalPointer<juce::TimeSliceThread> backgroundThread;

    struct ParameterListenersBackgroundTask : juce::TimeSliceClient
    {
        explicit ParameterListenersBackgroundTask (ParameterListeners& paramListeners, int timeSliceInterval);
        int useTimeSlice() override;
        ParameterListeners& listeners;
        const int interval;
    } backgroundTask;

    struct ParamInfo
    {
        const juce::RangedAudioParameter* paramCookie = nullptr;
        float value = 0.0f;
    };

    const size_t totalNumParams;
    std::vector<ParamInfo> paramInfoList { totalNumParams };

    static constexpr size_t actionSize = 16; // sizeof ([this, i = index] { callMessageThreadBroadcaster (i); })
    std::vector<Broadcaster<void()>> messageThreadBroadcasters { totalNumParams };
    using MessageThreadAction = juce::dsp::FixedSizeFunction<actionSize, void()>;
    moodycamel::ReaderWriterQueue<MessageThreadAction> messageThreadBroadcastQueue { totalNumParams };

    std::vector<Broadcaster<void()>> audioThreadBroadcasters { totalNumParams };
    using AudioThreadAction = juce::dsp::FixedSizeFunction<actionSize, void()>;
    moodycamel::ReaderWriterQueue<AudioThreadAction> audioThreadBroadcastQueue { totalNumParams };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterListeners)
};
JUCE_END_IGNORE_WARNINGS_MSVC
}
