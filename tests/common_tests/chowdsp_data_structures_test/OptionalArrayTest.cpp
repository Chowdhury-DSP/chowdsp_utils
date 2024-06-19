#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Optional Array Test", "[common][data-structures]")
{
    chowdsp::OptionalArray<std::string, 10> array {};
    REQUIRE (array.empty());
    REQUIRE (array.count_values() == 0);
    REQUIRE (! array.has_value (0));

    SECTION ("emplace()")
    {
        array.emplace (0, "test");
        array.emplace (1);

        REQUIRE (array[0].has_value());
        REQUIRE (array[0] == std::string { "test" });
        REQUIRE (array[1].has_value());
        REQUIRE (array[1] == std::string {});

        array.emplace (1, "test2");
        REQUIRE (array[1] == std::string { "test2" });
        REQUIRE (std::as_const (array)[1] == std::string { "test2" });

        REQUIRE (array[2] == std::nullopt);
        REQUIRE (std::as_const (array)[2] == std::nullopt);
    }

    SECTION ("erase()")
    {
        array.erase (0);
        REQUIRE (! array.has_value (0));

        array.emplace (0, "test");
        REQUIRE (array[0].has_value());

        array.erase (0);
        REQUIRE (! array.has_value (0));
    }

    SECTION ("iterate")
    {
        for (size_t i = 0; i < array.max_size; ++i)
            array.emplace (i, std::to_string (i));

        for (auto [idx, str] : chowdsp::enumerate (array))
        {
            str = "#" + str;
        }

        for (const auto& [idx, str] : chowdsp::enumerate (std::as_const (array)))
        {
            REQUIRE (str == std::string { "#" } + std::to_string (idx));
        }
    }
}
