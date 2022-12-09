#pragma once

namespace chowdsp
{
/** A stateful value that can be used to hold some non-parameter state */
template <typename T>
struct StateValue
{
    using element_type = T;

    /** Constructs the vaslue with a name and default value */
    StateValue (std::string_view valueName, T defaultValue)
        : name (valueName),
          defaultVal (defaultValue),
          val (defaultVal)
    {
    }

    /** Returns the value */
    T get() const noexcept { return val; }
    operator T() const noexcept { return get(); } // NOLINT(google-explicit-constructor): we want to be able to do implicit conversion

    /** Sets a new value */
    void set (T v)
    {
        if (v == val)
            return;

        val = v;
        changeBroadcaster();
    }

    StateValue& operator= (T v)
    {
        set (v);
        return *this;
    }

    /** Resets the value to its default state */
    void reset() { set (defaultVal); }

    template <typename Serializer>
    static void serialize (typename Serializer::SerializedType& serial, const StateValue& value)
    {
        Serializer::addChildElement (serial, value.name);
        Serializer::addChildElement (serial, Serialization::serialize<Serializer> (value.get()));
    }

    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType serial, StateValue& value)
    {
        T val;
        Serialization::deserialize<Serializer> (serial, val);
        value.set (val);
    }

    const std::string_view name;
    const T defaultVal;

private:
    T val;
    Broadcaster<void()> changeBroadcaster;

    template <typename ParameterState, typename NonParameterState, typename Serializer>
    friend class PluginState;
};
} // namespace chowdsp
