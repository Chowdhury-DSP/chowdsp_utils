#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>
#include <array>

struct Type0
{
    static constexpr std::string_view name = "type0";
};

struct Type1
{
    static constexpr std::string_view name = "type1";
};

struct Type2
{
    static constexpr std::string_view name = "type2";
};

TEST_CASE ("Types List Test", "[common][types]")
{
    SECTION ("For Each In Type List Test")
    {
        using List = types_list::TypesList<
            Type0,
            Type1,
            Type2>;

        static constexpr auto typeNames = []
        {
            std::array<std::string_view, List::count> names {};
            types_list::forEach<List> (
                [&names] (auto index)
                {
                    names[index] = List::AtIndex<index>::name;
                });
            return names;
        }();

        static_assert (typeNames[0] == "type0");
        static_assert (typeNames[1] == "type1");
        static_assert (typeNames[2] == "type2");
    }

    SECTION ("Contains Test")
    {
        static_assert (types_list::TypesList<std::string, int, float>::contains<int>);
        static_assert (! types_list::TypesList<std::string, int, float>::contains<double>);
    }

    SECTION ("Insert Front Test")
    {
        using BaseList = types_list::TypesList<Type0>;
        static_assert (std::is_same_v<types_list::TypesList<Type1, Type0>, BaseList::InsertFront<Type1>>);
        static_assert (std::is_same_v<types_list::TypesList<Type1, Type2, Type0>, BaseList::InsertFront<Type1, Type2>>);
    }

    SECTION ("Insert Back Test")
    {
        using BaseList = types_list::TypesList<Type0>;
        static_assert (std::is_same_v<types_list::TypesList<Type0, Type1>, BaseList::InsertBack<Type1>>);
        static_assert (std::is_same_v<types_list::TypesList<Type0, Type1, Type2>, BaseList::InsertBack<Type1, Type2>>);
    }

    SECTION ("Concatenate Test")
    {
        static_assert (std::is_same_v<types_list::TypesList<int, float>, types_list::TypesList<int>::Concatenate<types_list::TypesList<float>>>);
    }

    SECTION ("Tuple Test")
    {
        static_assert (std::is_same_v<types_list::TypesList<int, float>, types_list::TupleList<std::tuple<int, float>>>);
        static_assert (std::is_same_v<types_list::TypesList<int, float>::Types, std::tuple<int, float>>);
    }
}
