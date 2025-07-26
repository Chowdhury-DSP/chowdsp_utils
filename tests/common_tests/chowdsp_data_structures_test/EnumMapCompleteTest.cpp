#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Enum Map Complete Test", "[common][data-structures]")
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

    STATIC_REQUIRE (chowdsp::EnumMapComplete<Food, int>::size() == 8);

    SECTION ("Default Map")
    {
        chowdsp::EnumMapComplete<Food, int> map {};
        for (auto [key, val] : map)
            REQUIRE (val == 0);
    }

    SECTION ("Default Value Map")
    {
        chowdsp::EnumMapComplete<Food, int> map { 44 };
        for (auto [key, val] : map)
            REQUIRE (val == 44);
    }

    SECTION ("Construct/Access")
    {
        constexpr chowdsp::EnumMapComplete<Food, int> map {
            {
                { Food::Apple, 22 },
                { Food::Green_Beans, 23 },
            },
            20,
        };

        STATIC_REQUIRE (map.at (Food::Apple) == 22);
        STATIC_REQUIRE (map[Food::Green_Beans] == 23);
        STATIC_REQUIRE (std::as_const (map).at (Food::Apple) == 22);
        STATIC_REQUIRE (std::as_const (map)[Food::Green_Beans] == 23);
        STATIC_REQUIRE (map.at (Food::Banana) == 20);
    }

    SECTION ("Iteration")
    {
        chowdsp::EnumMapComplete<Food, int> map {
            { Food::Apple, 22 },
            { Food::Green_Beans, 23 },
        };

        size_t iter = 0;
        for (const auto& [key, val] : std::as_const (map))
        {
            if (iter == 0)
            {
                REQUIRE (key == Food::Apple);
                REQUIRE (val == 22);
            }
            else if (iter == 6)
            {
                REQUIRE (key == Food::Green_Beans);
                REQUIRE (val == 23);
            }
            else
            {
                REQUIRE (val == 0);
            }
            ++iter;
        }

        iter = 0;
        for (auto [key, val] : map)
            val = static_cast<int> (iter++);
        REQUIRE (map[Food::Apple] == 0);
        REQUIRE (map[Food::Egg] == 4);
        REQUIRE (map[Food::Green_Beans] == 6);
    }
}
