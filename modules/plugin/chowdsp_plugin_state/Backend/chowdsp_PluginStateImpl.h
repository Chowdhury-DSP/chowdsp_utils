#pragma once

namespace chowdsp
{
/**
 * Template type to hold a plugin's state.
 *
 * @tparam ParameterState       Struct containing all of the plugin's parameters as chowdsp::OptionalPointer's.
 * @tparam NonParameterState    Struct containing all of the plugin's non-parameter state as StateValue objects.
 * @tparam Serializer           A type that implements chowdsp::BaseSerializer (JSONSerializer by default)
 */
template <typename ParameterState, typename NonParameterState = NonParamState, typename Serializer = JSONSerializer>
class PluginStateImpl : public PluginState
{
    static_assert (std::is_base_of_v<ParamHolder, ParameterState>, "ParameterState must be a chowdsp::ParamHolder!");
    static_assert (std::is_base_of_v<NonParamState, NonParameterState>, "NonParameterState must be a chowdsp::NonParamState!");

public:
    /** Constructs a plugin state with no processor */
    explicit PluginStateImpl (juce::UndoManager* um = nullptr);

    /** Constructs the state and adds all the state parameters to the given processor */
    explicit PluginStateImpl (juce::AudioProcessor& proc, juce::UndoManager* um = nullptr);

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

    /** Returns the plugin non-parameter state */
    [[nodiscard]] NonParamState& getNonParameters() override;

    /** Returns the plugin non-parameter state */
    [[nodiscard]] const NonParamState& getNonParameters() const override;

    ParameterState params;
    NonParameterState nonParams;

private:
    Version pluginStateVersion {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginStateImpl)
};

} // namespace chowdsp

#include "chowdsp_PluginStateImpl.cpp"
