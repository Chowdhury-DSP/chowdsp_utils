// STL headers
#include <array>
#include <iostream>
#include <string_view>

#include <types_list/types_list.hpp>

// Construct a few random types:
struct Type1
{
    static constexpr std::string_view name { "Super cool type!" };
};

struct Type2
{
    static constexpr std::string_view name { "Less cool type!" };
};

struct Type3
{
    static constexpr std::string_view name { "Very boring type!" };
};

template <typename ListType>
void printList()
{
    // Iterate over the types in a list, and print their names
    static_assert (types_list::IsTypesList<ListType>);
    types_list::forEach<ListType> ([] (auto typeIndex)
                                   {
                                      using Type = typename ListType::template AtIndex<typeIndex>;
                                      std::cout << Type::name << std::endl; });
    std::cout << "----------------" << std::endl;
}

template <typename ListType>
constexpr auto getNames()
{
    // We know how may types are in the list at compile-time, so we can make a compile-time array!
    std::array<std::string_view, ListType::count> names;
    types_list::forEach<ListType> ([&names] (auto typeIndex)
                                   {
                                      using Type = typename ListType::template AtIndex<typeIndex>;
                                      names[typeIndex] = Type::name; });
    return names;
}

int main()
{
    // Construct a basic types list, and check if it contains a type:
    using List1 = types_list::TypesList<Type1, Type2>;
    static_assert (List1::contains<Type1>, "List1 should contain Type1");
    static_assert (! List1::contains<Type3>, "List1 should NOT contain Type3");
    printList<List1>();

    // Construct a types list using back-insertion
    using List2 = List1::InsertBack<Type3>;
    printList<List2>();

    // Construct a types list using front-insertion
    using List3 = List2::InsertFront<Type3>;
    printList<List3>();

    // Concatenate two types lists
    using List4 = List1::Concatenate<List3>;
    printList<List4>();
    static constexpr auto list4Names = getNames<List4>();
    static_assert (list4Names[0] == Type1::name, "First name is incorrect!");

    return 0;
}
