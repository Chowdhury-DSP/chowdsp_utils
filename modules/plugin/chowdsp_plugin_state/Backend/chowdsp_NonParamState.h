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
    void addStateValues (nonstd::span<StateValue<T>> newStateValues)
    {
        for (auto& val : newStateValues)
            values.push_back (&val);
        validateStateValues();
    }

    /** Adds more state values to this state. */
    template <typename ContainerType>
    void addStateValues (ContainerType& container)
    {
        for (auto& val : container)
            values.push_back (&val);
        validateStateValues();
    }

    /** Resets all the state values to their defaults */
    void reset();

    /** Custom serializer */
    static json serialize_json (const NonParamState& state);

    /** Custom deserializer */
    static void deserialize_json (const json& deserial, const NonParamState& state);

    /** Custom deserializer */
    static void legacy_deserialize (const json& deserial, const NonParamState& state);

    /** Assign this function to apply version streaming to your non-parameter state. */
    std::function<void (const Version&)> versionStreamingCallback = nullptr;

private:
    void validateStateValues() const;

    std::vector<StateValueBase*> values {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NonParamState)
};
} // namespace chowdsp
