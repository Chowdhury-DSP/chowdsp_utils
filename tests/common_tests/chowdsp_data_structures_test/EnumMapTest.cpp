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
        chowdsp::EnumMap<Food, int> map {
            { Food::Apple, 22 },
            { Food::Green_Beans, 23 },
        };

        REQUIRE (! map.empty());
        REQUIRE (map.size() == 2);
        REQUIRE (map.at (Food::Apple) == 22);
        REQUIRE (map[Food::Green_Beans] == 23);
        REQUIRE (std::as_const (map).at (Food::Apple) == 22);
        REQUIRE (std::as_const (map)[Food::Green_Beans] == 23);
    }
}
