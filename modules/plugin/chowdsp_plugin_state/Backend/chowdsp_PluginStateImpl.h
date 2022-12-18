#pragma once

namespace chowdsp
{
/** Empty struct that can be used as a placeholder for NonParameterState */
struct NullState
{
};

/**
 * Template type to hold a plugin's state.
 *
 * @tparam ParameterState       Struct containing all of the plugin's parameters as chowdsp::SmartPointer's. Structs may be nested.
 * @tparam NonParameterState    Struct containing all of the plugin's non-parameter state as StateValue onjects. Structs may be nested.
 * @tparam Serializer           A type that implements chowdsp::BaseSerializer (JSONSerializer by default)
 */
template <typename ParameterState, typename NonParameterState = NullState, typename Serializer = JSONSerializer>
class PluginStateImpl : public PluginState
{
    static_assert (std::is_base_of_v<ParamHolder, ParameterState>, "ParameterState must be a ParamHolder!");
    static_assert (std::is_aggregate_v<NonParameterState>, "NonParameterState must be an aggregate type!");

public:
    /** Constructs a plugin state with no processor */
    explicit PluginStateImpl (juce::UndoManager* um = nullptr, juce::TimeSliceThread* backgroundThread = nullptr);

    /** Constructs the state and adds all the state parameters to the given processor */
    explicit PluginStateImpl (juce::AudioProcessor& processor, juce::UndoManager* um = nullptr, juce::TimeSliceThread* backgroundThread = nullptr);

    /** Serializes the plugin state to the given MemoryBlock */
    void serialize (juce::MemoryBlock& data) const override;

    /** Deserializes the plugin state from the given MemoryBlock */
    void deserialize (const juce::MemoryBlock& data) override;

    /** Serializer */
    template <typename>
    static typename Serializer::SerializedType serialize (const PluginStateImpl& object);

    /** Deserializer */
    template <typename>
    static void deserialize (typename Serializer::DeserializedType serial, PluginStateImpl& object);

    ParameterState params;
    NonParameterState nonParams;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginStateImpl)
};

} // namespace chowdsp

#include "chowdsp_PluginStateImpl.cpp"
