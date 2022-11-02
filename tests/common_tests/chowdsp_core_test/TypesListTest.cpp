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

TEST_CASE ("Types List Test")
{
    SECTION ("For Each In Type List Test")
    {
        using List = chowdsp::TypesList<
            Type0,
            Type1,
            Type2>;

        static constexpr auto typeNames = []
        {
            std::array<std::string_view, List::count> names {};
            chowdsp::ForEachInTypeList<List>::doForEach (
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
        static_assert (chowdsp::TypesList<std::string, int, float>::contains<int>);
        static_assert (! chowdsp::TypesList<std::string, int, float>::contains<double>);
    }

    SECTION ("Insert Front Test")
    {
        using BaseList = chowdsp::TypesList<Type0>;
        static_assert (std::is_same_v<chowdsp::TypesList<Type1, Type0>, BaseList::InsertFront<Type1>>);
        static_assert (std::is_same_v<chowdsp::TypesList<Type1, Type2, Type0>, BaseList::InsertFront<Type1, Type2>>);
    }

    SECTION ("Insert Back Test")
    {
        using BaseList = chowdsp::TypesList<Type0>;
        static_assert (std::is_same_v<chowdsp::TypesList<Type0, Type1>, BaseList::InsertBack<Type1>>);
        static_assert (std::is_same_v<chowdsp::TypesList<Type0, Type1, Type2>, BaseList::InsertBack<Type1, Type2>>);
    }

    SECTION ("Concatenate Test")
    {
        static_assert (std::is_same_v<chowdsp::TypesList<int, float>, chowdsp::TypesList<int>::Concatenate<chowdsp::TypesList<float>>>);
    }

    SECTION ("Tuple Test")
    {
        static_assert (std::is_same_v<chowdsp::TypesList<int, float>, chowdsp::TupleTypeList<std::tuple<int, float>>>);
        static_assert (std::is_same_v<chowdsp::TypesList<int, float>::Types, std::tuple<int, float>>);
    }
}
