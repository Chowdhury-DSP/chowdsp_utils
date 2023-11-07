#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <array>
#include <map>

TEST_CASE ("Enumerate", "[common][data-structures]")
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

TEST_CASE ("Zip-Multi")
{
    SECTION ("ZIP with Self")
    {
        auto selfzip = [] (const auto& v)
        {
            int ct = 0;
            for (const auto& [a, b, c, d] : chowdsp::zip_multi (v, v, v, v))
            {
                REQUIRE (a == b);
                REQUIRE (a == c);
                REQUIRE (a == d);
                ct++;
            }
            REQUIRE (ct == (int) v.size());
        };
        selfzip (std::vector<int> { 1, 2, 3 });
        selfzip (std::vector<int>());
        selfzip (std::string ("hello world"));
        selfzip (std::array<int, 3> { 3, 2, 4 });
    }
    SECTION ("Simple Set of Vectors")
    {
        std::vector<int> v0 { 0, 1, 2 },
            v1 { 0, 2, 4 },
            v2 { 0, 3, 6 },
            v3 { 0, 4, 8 };
        for (const auto& [a, b, c, d] : chowdsp::zip_multi (v0, v1, v2, v3))
        {
            REQUIRE (a * 2 == b);
            REQUIRE (a * 3 == c);
            REQUIRE (a * 4 == d);
        }
    }

    SECTION ("Filling Arrays")
    {
        const std::array<int, 6> x_int { 0, 1, 2, 3, 4, 5 };
        std::array<float, 6> x_float {};
        std::array<double, 6> x_double {};
        for (auto [int_val, float_val, double_val] : chowdsp::zip_multi (x_int, x_float, x_double))
        {
            float_val = static_cast<float> (int_val);
            double_val = static_cast<double> (int_val);
        }
        for (auto [int_val, float_val, double_val] : chowdsp::zip_multi (x_int, x_float, x_double))
        {
            REQUIRE (juce::exactlyEqual ((float) int_val, float_val));
            REQUIRE (juce::exactlyEqual ((double) int_val, double_val));
        }
    }

    SECTION ("Varying Lengths")
    {
        std::vector<int> v0 { 0, 2, 4 }, v1 { 0, 1, 2, 3, 4, 5 };

        int ct { 0 };
        for (const auto& [a, b] : chowdsp::zip_multi (v0, v1))
        {
            REQUIRE (a == b * 2);
            ct++;
        }
        REQUIRE (ct == (int) std::min (v0.size(), v1.size()));

        ct = 0;
        for (const auto& [a, b] : chowdsp::zip_multi (v1, v0))
        {
            REQUIRE (a * 2 == b);
            ct++;
        }
        REQUIRE (ct == (int) std::min (v0.size(), v1.size()));

        std::array<int, 3> t2 {};
        for (const auto& [a, _, b] : chowdsp::zip_multi (v0, t2, "ace ventura"))
        {
            REQUIRE (a + 'a' == b);
        }
    }

    SECTION ("ZIP with Empty")
    {
        std::vector<int> test { 0, 1, 2 }, empty;
        for (const auto& [a, b] : chowdsp::zip_multi (test, empty))
        {
            juce::ignoreUnused (a, b);
            REQUIRE (false);
        }
        for (const auto& [a, b] : chowdsp::zip_multi (empty, test))
        {
            juce::ignoreUnused (a, b);
            REQUIRE (false);
        }
    }
}
