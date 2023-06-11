// STL headers
#include <array>
#include <iostream>
#include <string_view>

#include <boost/mp11.hpp>
namespace mp11 = boost::mp11;

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
    static_assert (mp11::mp_is_list<ListType>::value);

    // Iterate over the types in a list, and print their names
    mp11::mp_for_each<mp11::mp_iota_c<mp11::mp_size<ListType>::value>> (
        [] (auto I)
        {
            // I is mp_size_t<0>, mp_size_t<1>, ..., mp_size_t<N-1>
            std::cout << mp11::mp_at_c<ListType, I>::name << std::endl;
        });
    std::cout << "----------------" << std::endl;
}

template <typename ListType>
constexpr auto getNames()
{
    // We know how may types are in the list at compile-time, so we can make a compile-time array!
    std::array<std::string_view, mp11::mp_size<ListType>::value> names;
    mp11::mp_for_each<mp11::mp_iota_c<mp11::mp_size<ListType>::value>> (
        [&names] (auto I)
        {
            names[I] = mp11::mp_at_c<ListType, I>::name;
        });
    return names;
}

int main()
{
    // Construct a basic types list, and check if it contains a type:
    using List1 = mp11::mp_list<Type1, Type2>;
    static_assert (mp11::mp_contains<List1, Type1>::value, "List1 should contain Type1");
    static_assert (! mp11::mp_contains<List1, Type3>::value, "List1 should NOT contain Type3");
    printList<List1>();

    // Construct a types list using back-insertion
    using List2 = mp11::mp_push_back<List1, Type3>;
    printList<List2>();

    // Construct a types list using front-insertion
    using List3 = mp11::mp_push_front<List2, Type3>;
    printList<List3>();

    // Concatenate two types lists
    using List4 = mp11::mp_append<List1, List3>;
    printList<List4>();
    static constexpr auto list4Names = getNames<List4>();
    static_assert (list4Names[0] == Type1::name, "First name is incorrect!");

    return 0;
}
