#pragma once

namespace chowdsp
{
/** Struct that can be used as a list of types */
template <typename... Ts>
struct TypesList
{
    using Types = std::tuple<Ts...>;

    static constexpr auto count = std::tuple_size_v<Types>;

    template <size_t I>
    using AtIndex = std::tuple_element_t<I, Types>;

    template <typename T>
    static constexpr bool contains = (std::is_same_v<T, Ts> || ...);
};

/** Can be used to call a static function for every type in a type list */
template <typename TypesList, size_t typeIndex = 0, size_t numTypesLeft = TypesList::count>
struct ForEachInTypeList
{
    template <typename ForEachAction>
    static constexpr void doForEach (ForEachAction&& forEachAction)
    {
        forEachAction (std::integral_constant<std::size_t, typeIndex>());
        ForEachInTypeList<TypesList, typeIndex + 1, numTypesLeft - 1>::doForEach (std::forward<ForEachAction> (forEachAction));
    }
};

template <typename TypesList, size_t typeIndex>
struct ForEachInTypeList<TypesList, typeIndex, 0>
{
    template <typename ForEachAction>
    static constexpr void doForEach (ForEachAction&&)
    {
    }
};
} // namespace chowdsp
