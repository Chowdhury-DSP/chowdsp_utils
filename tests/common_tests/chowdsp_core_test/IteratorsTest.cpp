#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>
#include <array>
#include <map>

TEST_CASE ("Enumerate")
{
    SECTION ("Simple Vector")
    {
        std::vector<int> v { 0, 1, 2, 3 };
        for (const auto [idx, val] : chowdsp::enumerate (v))
        {
            REQUIRE ((int) idx == val);
        }

        std::vector<int> vOut;
        std::transform (v.begin(), v.end(), std::back_inserter (vOut), [] (auto a)
                        { return a * 2; });
        for (const auto [idx, val] : chowdsp::enumerate (vOut))
        {
            REQUIRE ((int) idx * 2 == val);
        }
    }

    SECTION ("Some other types")
    {
        std::string abcs = "abcdefg";
        for (const auto [idx, ch] : chowdsp::enumerate (abcs))
        {
            REQUIRE (ch == (int) idx + 'a');
        }

        std::array<char, 4> abcarr { 'd', 'e', 'f', 'g' };
        for (const auto [idx, ch] : chowdsp::enumerate (abcarr))
        {
            REQUIRE (ch == (int) idx + 'd');
        }
    }

    SECTION ("Empty Containers")
    {
        auto empty = [] (const auto& v)
        {
            bool never { true };
            for (const auto [i, j] : chowdsp::enumerate (v))
            {
                juce::ignoreUnused (i, j);
                never = false;
            }
            REQUIRE (never);
        };
        empty (std::vector<int>());
        empty (std::string());
        empty (std::array<int, 0>());
    }

    SECTION ("Enumerate Map")
    {
        std::map<std::string, std::string> m;
        m["hi"] = "there";
        m["zoo"] = "keeper";

        for (const auto [a, p] : chowdsp::enumerate (m))
        {
            auto [k, v] = p;
            if (a == 0)
            {
                REQUIRE (k == "hi");
                REQUIRE (v == "there");
            }
            if (a == 1)
            {
                REQUIRE (k == "zoo");
                REQUIRE (v == "keeper");
            }
        }
    }
}

TEST_CASE ("Zip")
{
    SECTION ("ZIP with Self")
    {
        auto selfzip = [] (const auto& v)
        {
            int ct = 0;
            for (const auto& [a, b] : chowdsp::zip (v, v))
            {
                REQUIRE (a == b);
                ct++;
            }
            REQUIRE (ct == (int) v.size());
        };
        selfzip (std::vector<int> { 1, 2, 3 });
        selfzip (std::vector<int>());
        selfzip (std::string ("hello world"));
        selfzip (std::array<int, 3> { 3, 2, 4 });
    }
    SECTION ("Simple Pair of Vectors")
    {
        std::vector<int> v0 { 0, 1, 2 }, v1 { 0, 2, 4 };
        for (const auto& [a, b] : chowdsp::zip (v0, v1))
        {
            REQUIRE (a * 2 == b);
        }
    }

    SECTION ("Varying Types")
    {
        std::vector<int> v0 { 0, 2, 4, 6, 8 };
        std::string msg = "acegi";
        for (const auto& [a, b] : chowdsp::zip (msg, v0))
        {
            REQUIRE (a == b + 'a');
        }
        for (const auto& [a, b] : chowdsp::zip (v0, msg))
        {
            REQUIRE (a + 'a' == b);
        }
    }

    SECTION ("Varying Lengths")
    {
        std::vector<int> v0 { 0, 2, 4 }, v1 { 0, 1, 2, 3, 4, 5 };

        int ct { 0 };
        for (const auto& [a, b] : chowdsp::zip (v0, v1))
        {
            REQUIRE (a == b * 2);
            ct++;
        }
        REQUIRE (ct == (int) std::min (v0.size(), v1.size()));

        ct = 0;
        for (const auto& [a, b] : chowdsp::zip (v1, v0))
        {
            REQUIRE (a * 2 == b);
            ct++;
        }
        REQUIRE (ct == (int) std::min (v0.size(), v1.size()));

        for (const auto& [a, b] : chowdsp::zip (v0, "ace ventura"))
        {
            REQUIRE (a + 'a' == b);
        }
    }

    SECTION ("ZIP with Empty")
    {
        std::vector<int> test { 0, 1, 2 }, empty;
        for (const auto& [a, b] : chowdsp::zip (test, empty))
        {
            juce::ignoreUnused (a, b);
            REQUIRE (false);
        }
        for (const auto& [a, b] : chowdsp::zip (empty, test))
        {
            juce::ignoreUnused (a, b);
            REQUIRE (false);
        }
    }
}
