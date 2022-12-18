#pragma once

namespace chowdsp
{
/** A stateful value that can be used to hold some non-parameter state */
template <typename T>
struct StateValue
{
    using element_type = T;

    /** Constructs the value with a name and default value */
    StateValue (std::string_view valueName, T defaultVal)
        : name (valueName),
          defaultValue (defaultVal),
          currentValue (defaultValue)
    {
    }

    /** Returns the value */
    T get() const noexcept { return currentValue; }
    operator T() const noexcept { return get(); } // NOLINT(google-explicit-constructor): we want to be able to do implicit conversion

    /** Sets a new value */
    void set (T v)
    {
        if (v == currentValue)
            return;

        currentValue = v;
        changeBroadcaster();
    }

    StateValue& operator= (T v)
    {
        set (v);
        return *this;
    }

    /** Resets the value to its default state */
    void reset() { set (defaultValue); }

    template <typename Serializer>
    static void serialize (typename Serializer::SerializedType& serial, const StateValue& value)
    {
        Serializer::addChildElement (serial, value.name);
        Serializer::addChildElement (serial, Serialization::serialize<Serializer> (value.get()));
    }

    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType serial, StateValue& value)
    {
        T val {};
        Serialization::deserialize<Serializer> (serial, val);
        value.set (val);
    }

    const std::string_view name;
    const T defaultValue;

    Broadcaster<void()> changeBroadcaster;

private:
    T currentValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StateValue)
};
} // namespace chowdsp
