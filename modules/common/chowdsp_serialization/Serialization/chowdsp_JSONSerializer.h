#pragma once

namespace chowdsp
{
class JSONSerializer
{
public:
    using SerializedType = json;

    template <typename T>
    static std::enable_if_t<std::is_arithmetic_v<T>, T>
        serialize (T x)
    {
        return x;
    }

    template <typename T>
    static constexpr auto IsString = std::is_same_v<T, std::string> || std::is_same_v<T, juce::String>;

    template <typename T>
    static std::enable_if_t<IsString<T>, T>
        serialize (const T& x)
    {
        return x;
    }

    template <typename T>
    static std::enable_if_t<MetaHelpers::IsContainer<T> && ! IsString<T>, SerializedType>
        serialize (const T& container)
    {
        auto j = json::array();
        for (const auto& val : container)
            j.push_back(serialize (val));

        return j;
    }

    template <typename T>
    static constexpr auto IsNotDirectlySerializable = ! std::is_arithmetic_v<T> && ! IsString<T> && ! MetaHelpers::IsContainer<T>;

    template <typename T>
    static std::enable_if_t<IsNotDirectlySerializable<T>, SerializedType> serialize (const T& t)
    {
        auto j = json::array();
        pfr::for_each_field (t, [&j] (const auto& field)
                             { j.push_back (serialize (field)); });

        return j;
    }

private:
    JSONSerializer() = default; // static use only
};
} // namespace chowdsp
