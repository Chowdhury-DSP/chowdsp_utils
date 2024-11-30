#pragma once
#include <chowdsp_serialization/Serialization/chowdsp_ByteSerializer.h>

namespace chowdsp
{
#ifndef DOXYGEN
struct StateValueBase
{
    explicit StateValueBase (std::string_view valueName) : name (valueName) {}
    StateValueBase (const StateValueBase&) = default;
    virtual ~StateValueBase() = default;

    virtual void reset() {}

    [[nodiscard]] virtual nlohmann::json serialize() const { return {}; }
    virtual void deserialize (const nlohmann::json&) {}

    [[nodiscard]] virtual size_t serialize (ChainedArenaAllocator&) const { return 0; }
    virtual void deserialize (nonstd::span<const std::byte>&) {}

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

    /** JSON Serializer */
    [[nodiscard]] nlohmann::json serialize() const override
    {
        return get();
    }

    /** JSON Deserializer */
    void deserialize (const nlohmann::json& deserial) override
    {
        set (deserial.get<element_type>());
    }

    /** Binary serializer */
    [[nodiscard]] size_t serialize (ChainedArenaAllocator& arena) const override
    {
        static constexpr auto is_span = TypeTraits::IsIterable<T> && ! TypeTraits::IsMapLike<T>;

        // Values need to track how many bytes they're serializing so the parent can know also.
        auto* serialize_num_bytes = arena.allocate<bytes_detail::size_type> (1, 1);

        size_t num_bytes = 0;
        if constexpr (std::is_same_v<T, json>)
        {
            num_bytes = serialize_string (get().dump(), arena);
        }
        else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
        {
            num_bytes = serialize_string (currentValue, arena);
        }
        else if constexpr (std::is_same_v<T, juce::String>)
        {
            num_bytes = serialize_string (toStringView (currentValue), arena);
        }
        else if constexpr (is_span)
        {
            num_bytes = serialize_span<typename T::value_type> (currentValue, arena);
        }
        else
        {
            num_bytes = serialize_object (get(), arena);
        }

        serialize_direct (serialize_num_bytes, num_bytes);
        return bytes_detail::sizeof_s + num_bytes;
    }

    void deserialize (nonstd::span<const std::byte>& data) override
    {
        static constexpr auto is_span = TypeTraits::IsIterable<T> && ! TypeTraits::IsMapLike<T>;

        [[maybe_unused]] const auto num_bytes = deserialize_direct<bytes_detail::size_type> (data);
        if constexpr (std::is_same_v<T, json>)
        {
            set (json::parse (deserialize_string (data)));
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            set (std::string { deserialize_string (data) });
        }
        else if constexpr (std::is_same_v<T, std::string_view>)
        {
            set (deserialize_string (data));
        }
        else if constexpr (std::is_same_v<T, juce::String>)
        {
            set (toString (deserialize_string (data)));
        }
        else if constexpr (is_span)
        {
            deserialize_span<typename T::value_type> (currentValue, data);
        }
        else
        {
            set (deserialize_object<element_type> (data));
        }
    }

    const element_type defaultValue;

private:
    T currentValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StateValue)
};
} // namespace chowdsp
