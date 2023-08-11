#pragma once

namespace chowdsp
{
/** Container for holding non-parameter state values. */
class NonParamState
{
public:
    NonParamState() = default;

    NonParamState (NonParamState&&) noexcept = default;
    NonParamState& operator= (NonParamState&&) noexcept = default;

    /** Adds more state values to this state. */
    void addStateValues (const std::initializer_list<StateValueBase*>& newStateValues);

    /** Adds more state values to this state. */
    template <typename T>
    void addStateValues (nonstd::span<StateValue<T>> newStateValues);

    /** Custom serializer */
    template <typename Serializer>
    static typename Serializer::SerializedType serialize (const NonParamState& state);

    /** Custom deserializer */
    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType deserial, const NonParamState& state);

    /** Assign this function to apply version streaming to your non-parameter state. */
    std::function<void (const Version&)> versionStreamingCallback = nullptr;

private:
    void validateStateValues() const;

    std::vector<StateValueBase*> values;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NonParamState)
};
} // namespace chowdsp

#include "chowdsp_NonParamState.cpp"
