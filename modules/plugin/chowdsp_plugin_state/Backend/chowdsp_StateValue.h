#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
struct StateValueBase
{
    explicit StateValueBase (std::string_view valueName) : name (valueName) {}
    StateValueBase (const StateValueBase&) = default;
    virtual ~StateValueBase() = default;

    virtual void reset() {}

#if CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
    virtual void serialize (JSONSerializer::SerializedType&) const {}
    virtual void deserialize (JSONSerializer::DeserializedType) {}
#else
    [[nodiscard]] virtual nlohmann::json serialize () const { return {}; }
    virtual void deserialize (const nlohmann::json&) {}
#endif

    const std::string_view name {};
    Broadcaster<void()> changeBroadcaster {};
};
#endif

/** A stateful value that can be used to hold some non-parameter state */
template <typename T, typename element_type_ = T>
struct StateValue : StateValueBase
{
    using element_type = element_type_;

    /** Default constructor */
    StateValue()
        : StateValueBase ({}),
          defaultValue (element_type {}),
          currentValue (defaultValue)
    {
    }

    /** Constructs the value with a name and default value */
    StateValue (std::string_view valueName, element_type defaultVal)
        : StateValueBase (valueName),
          defaultValue (defaultVal),
          currentValue (defaultValue)
    {
    }

    StateValue (StateValue&&) noexcept = default;
    StateValue& operator= (StateValue&&) noexcept = default;

    /** Returns the value */
    element_type get() const noexcept { return currentValue; }
    operator element_type() const noexcept { return get(); } // NOSONAR NOLINT(google-explicit-constructor): we want to be able to do implicit conversion

    /** Sets a new value */
    void set (element_type v)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
        if (v == currentValue)
            return;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        currentValue = v;
        changeBroadcaster();
    }

    StateValue& operator= (element_type v)
    {
        set (v);
        return *this;
    }

    /** Resets the value to its default state */
    void reset() override { set (defaultValue); }

#if CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
    /** JSON Serializer */
    void serialize (JSONSerializer::SerializedType& serial) const override
    {
        serialize<JSONSerializer> (serial, *this);
    }

    /** JSON Deserializer */
    void deserialize (JSONSerializer::DeserializedType deserial) override
    {
        deserialize<JSONSerializer> (deserial, *this);
    }
#else
    /** JSON Serializer */
    [[nodiscard]] nlohmann::json serialize () const override
    {
        return get();
    }

    /** JSON Deserializer */
    void deserialize (const nlohmann::json& deserial) override
    {
        set (deserial.get<element_type_>());
    }
#endif

    const element_type defaultValue;

private:
#if CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
    template <typename Serializer>
    static void serialize (typename Serializer::SerializedType& serial, const StateValue& value)
    {
        Serializer::addChildElement (serial, value.name);
        Serializer::addChildElement (serial, Serialization::serialize<Serializer> (value.get()));
    }

    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType deserial, StateValue& value)
    {
        element_type val {};
        Serialization::deserialize<Serializer> (deserial, val);
        value.set (val);
    }
#endif

    T currentValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StateValue)
};
} // namespace chowdsp
