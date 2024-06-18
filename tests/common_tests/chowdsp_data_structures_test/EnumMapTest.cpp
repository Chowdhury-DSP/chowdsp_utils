#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Enum Map Test", "[common][data-structures]")
{
    enum class Food
    {
        Apple,
        Banana,
        Carrot,
        Donut,
        Egg,
        Fry_Bread,
        Green_Beans,
        Huevos_Rancheros
    };

    STATIC_REQUIRE (chowdsp::EnumMap<Food, int>::max_size() == 8);

    SECTION ("Empty Map")
    {
        chowdsp::EnumMap<Food, int> map {};
        REQUIRE (map.empty());
        REQUIRE (map.size() == 0); // NOLINT
    }

    SECTION ("Construct/Access")
    {
        constexpr chowdsp::EnumMap<Food, int> map {
            { Food::Apple, 22 },
            { Food::Green_Beans, 23 },
        };

        STATIC_REQUIRE (! map.empty());
        STATIC_REQUIRE (map.size() == 2);
        STATIC_REQUIRE (*map.at (Food::Apple) == 22);
        STATIC_REQUIRE (map[Food::Green_Beans] == 23);
        STATIC_REQUIRE (*std::as_const (map).at (Food::Apple) == 22);
        STATIC_REQUIRE (std::as_const (map)[Food::Green_Beans] == 23);
        STATIC_REQUIRE (map.at (Food::Banana) == std::nullopt);
    }

    SECTION ("Insertion/Erasure")
    {
        chowdsp::EnumMap<Food, int> map {};
        map.insert_or_assign (Food::Apple, 22);
        map.emplace (Food::Green_Beans, 23);
        map.emplace (Food::Egg);

        REQUIRE (! map.empty());
        REQUIRE (map.size() == 3);
        REQUIRE (*map.at (Food::Apple) == 22);
        REQUIRE (map[Food::Green_Beans] == 23);
        REQUIRE (map[Food::Egg] == 0);
        REQUIRE (*std::as_const (map).at (Food::Apple) == 22);
        REQUIRE (std::as_const (map)[Food::Green_Beans] == 23);
        REQUIRE (map.at (Food::Banana) == std::nullopt);

        map.erase (Food::Green_Beans);
        REQUIRE (map.at (Food::Green_Beans) == std::nullopt);

        REQUIRE (map.contains (Food::Apple));
        REQUIRE (! map.contains (Food::Green_Beans));

        map.clear();
        REQUIRE (map.empty());
    }

    SECTION ("Iteration")
    {
        chowdsp::EnumMap<Food, int> map {
            { Food::Apple, 22 },
            { Food::Green_Beans, 23 },
        };

        size_t iter = 0;
        for (const auto& [key, val] : std::as_const (map))
        {
            jassert (iter < 2);
            if (iter == 0)
            {
                REQUIRE (key == Food::Apple);
                REQUIRE (val == 22);
            }
            else
            {
                REQUIRE (key == Food::Green_Beans);
                REQUIRE (val == 23);
            }
            ++iter;
        }

        iter = 0;
        for (auto [key, val] : map)
            val = static_cast<int> (iter++);
        REQUIRE (map[Food::Apple] == 0);
        REQUIRE (map[Food::Green_Beans] == 1);
    }
}
