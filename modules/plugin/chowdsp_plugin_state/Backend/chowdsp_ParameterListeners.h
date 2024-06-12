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
#if __has_include(<readerwriterqueue.h>)
#include <readerwriterqueue.h>
#else
#include "../../../dsp/chowdsp_dsp_data_structures/third_party/moodycamel/readerwriterqueue.h"
#endif
#endif

namespace chowdsp
{
/** Enum to specify which thread a parameter listener callback should happen on. */
enum class ParameterListenerThread
{
    MessageThread,
    AudioThread,
};

/** Utility class to manage a set of parameter listeners. */
class ParameterListeners : private juce::Timer,
                           private juce::AudioProcessorParameter::Listener
{
public:
    /** Initialises the listeners with a set of parameters. */
    explicit ParameterListeners (ParamHolder& parameters,
                                 const juce::AudioProcessor* parentProcessor = nullptr,
                                 int intervalMilliseconds = 20);
    ~ParameterListeners() override;

    /**
     * Runs any queued listeners on the audio thread.
     *
     * If you're using chowdsp::PluginBase, this will get called automatically,
     * otherwise, users must call this in the audio callback themselves!
     */
    void callAudioThreadBroadcasters();

    /**
     * If you are changing a bunch of parameters at once (e.g. preset loading),
     * and want the MessageThread changes to propagate immediately, this function
     * will do that for you!
     */
    void updateBroadcastersFromMessageThread();

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

    void timerCallback() override;
    void parameterValueChanged (int, float) override;
    void parameterGestureChanged (int, bool) override {}

    struct ParamInfo
    {
        juce::RangedAudioParameter* paramCookie = nullptr;
        float value = 0.0f;
    };

    const size_t totalNumParams;
    std::vector<ParamInfo> paramInfoList { totalNumParams };

    std::vector<Broadcaster<void()>> messageThreadBroadcasters { totalNumParams };

    static constexpr size_t actionSize = 16; // sizeof ([this, i = index] { callMessageThreadBroadcaster (i); })
    std::vector<Broadcaster<void()>> audioThreadBroadcasters { totalNumParams };
    using AudioThreadAction = juce::dsp::FixedSizeFunction<actionSize, void()>;
    moodycamel::ReaderWriterQueue<AudioThreadAction> audioThreadBroadcastQueue { totalNumParams };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterListeners)
};
} // namespace chowdsp
