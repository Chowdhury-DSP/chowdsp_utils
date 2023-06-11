#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace types_list
{
/** Can be used to call a lambda for every type in a type list */
template <typename TypesList, typename ForEachAction, size_t typeIndex = 0, size_t numTypesLeft = TypesList::count>
static constexpr void forEach (ForEachAction&& forEachAction)
{
    if constexpr (numTypesLeft > 0)
    {
        forEachAction (std::integral_constant<std::size_t, typeIndex>());
        forEach<TypesList, ForEachAction, typeIndex + 1, numTypesLeft - 1> (
            std::forward<ForEachAction> (forEachAction));
    }
}

namespace detail
{
    template <typename TypesList, typename OtherListType, size_t typeIndex = 0, size_t numTypesLeft = OtherListType::count>
    struct ConcatHelper
    {
        using ConcatenatedListType = typename ConcatHelper<typename TypesList::template InsertBack<
                                                               typename OtherListType::template AtIndex<typeIndex>>,
                                                           OtherListType,
                                                           typeIndex + 1,
                                                           numTypesLeft - 1>::ConcatenatedListType;
    };

    template <typename TypesList, typename OtherListType, size_t typeIndex>
    struct ConcatHelper<TypesList, OtherListType, typeIndex, 0>
    {
        using ConcatenatedListType = TypesList;
    };

    template <typename TypesList, typename TupleType, size_t typeIndex = 0, size_t numTypesLeft = std::tuple_size<TupleType>()>
    struct TupleHelpers
    {
        using TupleTypesList = typename TupleHelpers<typename TypesList::template InsertBack<std::tuple_element_t<typeIndex, TupleType>>,
                                                     TupleType,
                                                     typeIndex + 1,
                                                     numTypesLeft - 1>::TupleTypesList;
    };

    template <typename TypesList, typename TupleType, size_t typeIndex>
    struct TupleHelpers<TypesList, TupleType, typeIndex, 0>
    {
        using TupleTypesList = TypesList;
    };

    template <typename TypesList, typename Type>
    static constexpr int indexOf()
    {
        int result = -1;
        forEach<TypesList> (
            [&result] (auto typeIndex)
            {
                using TypeAtIndex = typename TypesList::template AtIndex<typeIndex>;
                if constexpr (std::is_same_v<Type, TypeAtIndex>)
                    result = (int) typeIndex;
            });
        return result;
    }
} // namespace detail

/** Template object that can be used as a list of types */
template <typename... Ts>
struct TypesList
{
    /** All the types in the list as a tuple. */
    using Types = std::tuple<Ts...>;

    /** The number of types in the list. */
    static constexpr auto count = std::tuple_size_v<Types>;

    /** Can be used to access the type at a given index in the list. */
    template <size_t I>
    using AtIndex = std::tuple_element_t<I, Types>;

    /** Returns the index of a given type in the list. Returns -1 if the type is not present in the list. */
    template <typename T>
    static constexpr int IndexOf = detail::indexOf<TypesList, T>();

    /** True if the argument type is present in the list. */
    template <typename T>
    static constexpr bool contains = (std::is_same_v<T, Ts> || ...);

    /** Creates a new type list by inserting the new types into this list. */
    template <typename... NewTs>
    using InsertFront = TypesList<NewTs..., Ts...>;

    /** Creates a new type list by inserting the new types into this list. */
    template <typename... NewTs>
    using InsertBack = TypesList<Ts..., NewTs...>;

    /** Concatenates this list type with another */
    template <typename OtherListType>
    using Concatenate = typename detail::ConcatHelper<TypesList, OtherListType>::ConcatenatedListType;
};

/** Constructs a TypesList from a tuple */
template <typename TupleType>
using TupleList = typename detail::TupleHelpers<TypesList<>, TupleType>::TupleTypesList;

namespace detail
{
    template <typename MaybeTypesList>
    struct IsTypesListHelper
    {
        static constexpr bool value = false;
    };

    template <typename... Ts>
    struct IsTypesListHelper<TypesList<Ts...>>
    {
        static constexpr bool value = true;
    };
} // namespace detail

/** True if the template argument is a TypesList */
template <typename MaybeTypesList>
constexpr bool IsTypesList = detail::IsTypesListHelper<MaybeTypesList>::value;

namespace detail
{
    template <typename T>
    struct TypesWrapperHelper
    {
        using Types = T;
    };

    template <typename... Ts>
    struct TypesWrapperHelper<TypesList<Ts...>>
    {
        using Types = typename TypesList<Ts...>::Types;
    };
}

/**
 * If the template argument is a TypesList, the aliased type will
 * be a tuple containing all the types in the list. Otherwise the
 * aliased type will be the same as the template argument.
 */
template <typename T>
using TypesWrapper = typename detail::TypesWrapperHelper<T>::Types;
} // namespace types_list
