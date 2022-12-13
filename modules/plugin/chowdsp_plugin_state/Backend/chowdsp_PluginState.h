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
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324) // struct was padded warning

/** An empty plugin state object */
struct NullState
{
};

struct NameTag
{
    const std::string name;

    template <typename Serializer>
    static typename Serializer::SerializedType serialize (const NameTag& object)
    {
        return Serializer::serializeString (object.name);
    }

    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType serial, NameTag& object)
    {
        object.name = Serializer::template deserializeString<std::string> (serial);
    }
};

/**
 * Template type to hold a plugin's state.
 *
 * @tparam ParameterState       Struct containing all of the plugin's parameters as chowdsp::SmartPointer's. Structs may be nested.
 * @tparam NonParameterState    Struct containing all of the plugin's non-parameter state as StateValue onjects. Structs may be nested.
 * @tparam Serializer           A type that implements chowdsp::BaseSerializer (JSONSerializer by default)
 */
template <typename ParameterState, typename NonParameterState = NullState, typename Serializer = JSONSerializer>
class PluginState : private juce::HighResolutionTimer, // @TODO: maybe we should use a TimeSliceThread instead?
                    private juce::AsyncUpdater
{
    static_assert (PluginStateHelpers::ContainsOnlyParamPointers<ParameterState>,
                   "ParameterState must contain only chowdsp::SmartPointer<> of parameter types,"
                   " or structs containing those types!");

    static_assert (PluginStateHelpers::ContainsOnlyStateValues<NonParameterState>,
                   "NonParameterState must only contain chowdsp::StateValue types or structs containing those types!");

public:
    /** Constructs a plugin state with no processor */
    explicit PluginState (juce::UndoManager* um = nullptr);

    /** Constructs the state and adds all the state parameters to the given processor */
    explicit PluginState (juce::AudioProcessor& processor, juce::UndoManager* um = nullptr);

    /** Destructor */
    ~PluginState() override;

    /** Accesses a parameter with a given path. */
    template <typename ParameterType>
    ParameterType& getParameter (const ParameterPath& path);

    /** Accesses a parameter with a given path. */
    template <typename ParameterType>
    const ParameterType& getParameter (const ParameterPath& path) const;

    /** Serializes the plugin state to the given MemoryBlock */
    void serialize (juce::MemoryBlock& data) const;

    /** Deserializes the plugin state from the given MemoryBlock */
    void deserialize (const juce::MemoryBlock& data);

    /** Serializer */
    template <typename>
    static typename Serializer::SerializedType serialize (const PluginState& object);

    /** Deserializer */
    template <typename>
    static void deserialize (typename Serializer::DeserializedType serial, PluginState& object);

    /**
     * Adds a parameter listener which will be called on either the message
     * thread or the audio thread (you choose!). Listeners should have the
     * signature void().
     */
    template <typename ParamType, typename... ListenerArgs>
    [[nodiscard]] ScopedCallback addParameterListener (const ParamType& param, bool listenOnMessageThread, ListenerArgs&&... args);

    /**
     * Adds a listener to some field of the plugin's non-parameter state.
     * The listener will be called on whichever thread the state value is
     * mutated on. Listeners should have the signature void().
     */
    template <typename NonParamType, typename... ListenerArgs>
    [[nodiscard]] ScopedCallback addNonParameterListener (StateValue<NonParamType>& nonParam, ListenerArgs&&... args);

    /**
     * Runs parameter broadcasters synchronously.
     * This method is intended to be called from the audio thread.
     */
    void callAudioThreadBroadcasters();

    /**
     * Runs parameter broadcasters synchronously.
     * This method is intended to be called from the message thread.
     */
    void callMessageThreadBroadcasters();

    ParameterState params;
    NonParameterState nonParams;
    juce::UndoManager* undoManager = nullptr;

private:
    void callMessageThreadBroadcaster (size_t index);
    void callAudioThreadBroadcaster (size_t index);

    void hiResTimerCallback() override;
    void handleAsyncUpdate() override;

    // Tools for managing parameter changes
    struct ParamInfo
    {
        const juce::RangedAudioParameter* paramCookie = nullptr;
        float value = 0.0f;
    };

    static constexpr auto totalNumParams = (size_t) PluginStateHelpers::ParamCount<ParameterState>;
    std::array<ParamInfo, totalNumParams> paramInfoList;

    static constexpr size_t actionSize = 16; // sizeof ([this, i = index] { callMessageThreadBroadcaster (i); })
    std::array<Broadcaster<void()>, totalNumParams> messageThreadBroadcasters;
    using MessageThreadAction = juce::dsp::FixedSizeFunction<actionSize, void()>;
    moodycamel::ReaderWriterQueue<MessageThreadAction> messageThreadBroadcastQueue { totalNumParams };

    std::array<Broadcaster<void()>, totalNumParams> audioThreadBroadcasters;
    using AudioThreadAction = juce::dsp::FixedSizeFunction<actionSize, void()>;
    moodycamel::ReaderWriterQueue<AudioThreadAction> audioThreadBroadcastQueue { totalNumParams };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginState)
};

JUCE_END_IGNORE_WARNINGS_MSVC

} // namespace chowdsp

#include "chowdsp_PluginState.cpp"
