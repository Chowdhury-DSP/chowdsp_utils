#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Small Map Test", "[common][data-structures]")
{
    SECTION ("String -> String Map")
    {
        using StringMap = chowdsp::SmallMap<std::string, std::string>;
        StringMap map {
            {
                { "one", "1" },
                { "two", "2" },
                { "three", "3" },
            }
        };

        SECTION ("Accessors")
        {
            REQUIRE (map.empty() == false);
            REQUIRE (map.size() == 3);

            REQUIRE (*map["one"] == "1");
            REQUIRE (*map["two"] == "2");
            REQUIRE (*map["three"] == "3");
            REQUIRE (map["four"] == nullptr);

            REQUIRE (map.contains ("three") == true);
            REQUIRE (map.contains ("blah") == false);
        }

        SECTION ("Insert/Assign Copy")
        {
            const std::string zero { "0" };
            map.insert_or_assign ("zero", zero);
            REQUIRE (*map["zero"] == "0");

            const std::string zero_blank { " " };
            map.insert_or_assign ("zero", zero_blank);
            REQUIRE (*map["zero"] == " ");
        }

        SECTION ("Insert/Assign Move")
        {
            map.insert_or_assign ("zero", "0");
            REQUIRE (*map["zero"] == "0");

            map.insert_or_assign ("zero", " ");
            REQUIRE (*map["zero"] == " ");
        }
    }

    SECTION ("String -> Int Map")
    {
        chowdsp::SmallMap<std::string, int> map {
            {
                { "one", 1 },
                { "two", 2 },
                { "three", 3 },
            }
        };

        SECTION ("Iterate")
        {
            int sum = 0;
            for (auto [key, value] : map)
            {
                if (key == "one")
                    REQUIRE (value == 1);
                else if (key == "two")
                    REQUIRE (value == 2);
                else if (key == "three")
                    REQUIRE (value == 3);
                else
                    REQUIRE (false);
                sum += value;
            }
            REQUIRE (sum == 6);
        }

        SECTION ("Erase")
        {
            REQUIRE (*map["one"] == 1);
            REQUIRE (*map["two"] == 2);
            REQUIRE (*map["three"] == 3);

            map.erase ("two");

            REQUIRE (*map["one"] == 1);
            REQUIRE (map["two"] == nullptr);
            REQUIRE (*map["three"] == 3);
        }
    }
}
