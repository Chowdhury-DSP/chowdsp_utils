#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Small Vector Test", "[common][data-structures]")
{
    STATIC_REQUIRE (sizeof (chowdsp::SmallVector<int, 8>) < 2 * sizeof (std::array<int, 8>));

    SECTION ("Push/Pop Test")
    {
        chowdsp::SmallVector<int, 2> vec;
        vec.push_back (1);
        vec.push_back (2);
        vec.push_back (3);
        vec.push_back (4);
        vec.push_back (5);
        REQUIRE (vec[0] == 1);
        REQUIRE (vec[1] == 2);
        REQUIRE (vec[2] == 3);
        REQUIRE (vec[3] == 4);
        REQUIRE (std::as_const (vec)[4] == 5);
        REQUIRE (vec.size() == 5);
        REQUIRE (vec.capacity() >= 5);

        vec.pop_back();
        vec.shrink_to_fit();
        REQUIRE (vec[0] == 1);
        REQUIRE (vec[1] == 2);
        REQUIRE (vec[2] == 3);
        REQUIRE (vec[3] == 4);
        REQUIRE (vec.size() == 4);
        REQUIRE (vec.capacity() >= 4);

        vec.pop_back();
        vec.pop_back();
        vec.shrink_to_fit();

        REQUIRE (vec[0] == 1);
        REQUIRE (std::as_const (vec)[1] == 2);
        REQUIRE (vec.size() == 2);
        REQUIRE (vec.capacity() == 2);

        vec.pop_back();
        vec.shrink_to_fit();
        REQUIRE (vec.size() == 1);
        REQUIRE (vec.capacity() == 2);
    }

    SECTION ("Push/Pop String Test")
    {
        chowdsp::SmallVector<std::string, 2> vec;
        REQUIRE (vec.empty());

        std::string avs[] = { "Joe", "Peter", "Milan", "Alex", "Ray" };

        vec.push_back (avs[0]);
        vec.emplace_back ("Peter");
        vec.push_back (avs[2]);
        vec.push_back (avs[3]);
        vec.emplace_back ("Ray");
        REQUIRE (vec.at (0) == avs[0]);
        REQUIRE (vec.at (1) == avs[1]);
        REQUIRE (vec.at (2) == avs[2]);
        REQUIRE (vec.at (3) == avs[3]);
        REQUIRE (std::as_const (vec).at (4) == avs[4]);
        REQUIRE (vec.size() == 5);
        REQUIRE (! vec.empty());
        REQUIRE (vec.capacity() >= 5);

        vec.pop_back();
        vec.shrink_to_fit();
        REQUIRE (vec[0] == "Joe");
        REQUIRE (vec[1] == "Peter");
        REQUIRE (vec[2] == "Milan");
        REQUIRE (vec[3] == "Alex");
        REQUIRE (vec.size() == 4);
        REQUIRE (vec.capacity() >= 4);

        vec.pop_back();
        vec.pop_back();
        vec.shrink_to_fit();

        REQUIRE (vec.at (0) == "Joe");
        REQUIRE (std::as_const (vec).at (1) == "Peter");
        REQUIRE (vec.size() == 2);
        REQUIRE (vec.capacity() == 2);

        vec.emplace_back ("Milan");
        REQUIRE (std::as_const (vec)[2] == "Milan");
        REQUIRE (vec.size() == 3);

        vec.pop_back();
        vec.pop_back();
        vec.shrink_to_fit();
        REQUIRE (vec.size() == 1);
        REQUIRE (vec.capacity() == 2);
    }

    SECTION ("Loop Test")
    {
        for (int size : { 8, 16 })
        {
            chowdsp::SmallVector<int, 8> vec;
            vec.resize ((size_t) size);

            {
                int count = 0;
                for (auto& v : vec)
                    v = count++;
            }

            for (const auto [n, v] : chowdsp::enumerate (std::as_const (vec)))
                REQUIRE ((int) n == v);
        }
    }

    SECTION ("Reverse Loop Test")
    {
        for (int size : { 8, 16 })
        {
            chowdsp::SmallVector<int, 8> vec;
            vec.resize ((size_t) size);

            {
                int count = 0;
                for (auto& v : chowdsp::reverse (vec))
                    v = count++;
            }

            for (const auto [n, v] : chowdsp::enumerate (chowdsp::reverse (std::as_const (vec))))
                REQUIRE ((int) n == v);
        }
    }

    SECTION ("Construct with default")
    {
        chowdsp::SmallVector<std::string, 4> vec { 10 };
        REQUIRE (vec.size() == 10);
        for (const auto& v : vec)
            REQUIRE (v.empty());
    }

    SECTION ("Construct with value")
    {
        chowdsp::SmallVector<std::string, 4> vec { 3, "blah" };
        REQUIRE (vec.size() == 3);
        for (const auto& v : vec)
            REQUIRE (v == "blah");
    }

    SECTION ("Construct with range (small)")
    {
        std::array<std::string, 4> beatles { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 4> vec { beatles.begin(), beatles.end() };
        REQUIRE (vec.size() == beatles.size());
        for (const auto [ref, actual] : chowdsp::zip (beatles, vec))
            REQUIRE (ref == actual);
    }

    SECTION ("Construct with range (large)")
    {
        std::array<std::string, 4> beatles { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 2> vec { beatles.begin(), beatles.end() };
        REQUIRE (vec.size() == beatles.size());
        for (const auto [ref, actual] : chowdsp::zip (beatles, vec))
            REQUIRE (ref == actual);
    }

    SECTION ("Construct with range (small)")
    {
        std::array<std::string, 4> beatles { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 4> vec { "john", "paul", "george", "ringo" };
        REQUIRE (vec.size() == beatles.size());
        for (const auto [ref, actual] : chowdsp::zip (beatles, vec))
            REQUIRE (ref == actual);
    }

    SECTION ("Construct with range (large)")
    {
        std::array<std::string, 4> beatles { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 2> vec { "john", "paul", "george", "ringo" };
        REQUIRE (vec.size() == beatles.size());
        for (const auto [ref, actual] : chowdsp::zip (beatles, vec))
            REQUIRE (ref == actual);
    }

    SECTION ("Copy Construction Test")
    {
        chowdsp::SmallVector<std::string, 4> vec { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 4> vec2 { vec };
        REQUIRE (vec.size() == vec2.size());
        for (const auto [ref, actual] : chowdsp::zip (vec, vec2))
            REQUIRE (ref == actual);
    }

    SECTION ("Copy Assign Test")
    {
        chowdsp::SmallVector<std::string, 2> vec { "john", "paul", "george", "ringo" };
        const auto vec2 = vec.operator= (vec);
        REQUIRE (vec.size() == vec2.size());
        for (const auto [ref, actual] : chowdsp::zip (vec, vec2))
            REQUIRE (ref == actual);
    }

    SECTION ("Move Construction Test")
    {
        std::array<std::string, 4> beatles { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 4> vec { beatles.begin(), beatles.end() };
        chowdsp::SmallVector<std::string, 4> vec2 { std::move (vec) };
        REQUIRE (beatles.size() == vec2.size());
        for (const auto [ref, actual] : chowdsp::zip (beatles, vec2))
            REQUIRE (ref == actual);
    }

    SECTION ("Move Assign Test")
    {
        std::array<std::string, 4> beatles { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 2> vec { beatles.begin(), beatles.end() };
        const auto vec2 = std::move (vec);
        REQUIRE (beatles.size() == vec2.size());
        for (const auto [ref, actual] : chowdsp::zip (beatles, vec2))
            REQUIRE (ref == actual);
    }

    SECTION ("Front/Back/Data (small) Test")
    {
        chowdsp::SmallVector<std::string, 4> vec { "john", "paul", "george", "ringo" };
        REQUIRE (vec.front() == "john");
        REQUIRE (std::as_const (vec).front() == "john");
        REQUIRE (vec.back() == "ringo");
        REQUIRE (std::as_const (vec).back() == "ringo");
        REQUIRE (*vec.data() == "john");
        REQUIRE (*std::as_const (vec).data() == "john");
    }

    SECTION ("Front/Back/Data (large) Test")
    {
        chowdsp::SmallVector<std::string, 2> vec { "john", "paul", "george", "ringo" };
        REQUIRE (vec.front() == "john");
        REQUIRE (std::as_const (vec).front() == "john");
        REQUIRE (vec.back() == "ringo");
        REQUIRE (std::as_const (vec).back() == "ringo");
        REQUIRE (*vec.data() == "john");
        REQUIRE (*std::as_const (vec).data() == "john");
    }

    SECTION ("Reserve Test")
    {
        chowdsp::SmallVector<int, 4> vec {};
        REQUIRE (vec.capacity() == 4);

        vec.reserve (3);
        REQUIRE (vec.capacity() == 4);

        vec.reserve (10);
        REQUIRE (vec.capacity() == 10);

        vec.shrink_to_fit();
        REQUIRE (vec.capacity() == 4);
    }

    SECTION ("Clear Test")
    {
        chowdsp::SmallVector<int, 4> vec { 1, 2, 3 };
        vec.resize (3);
        REQUIRE (vec[2] == 3);

        vec.clear();
        REQUIRE (vec.empty());

        vec.push_back (0);
        vec.push_back (1);
        vec.push_back (2);
        vec.push_back (3);
        vec.push_back (4);
        REQUIRE (vec.size() == 5);

        vec.clear();
        REQUIRE (vec.empty());
    }

    SECTION ("Clear w/ Destruction Test")
    {
        chowdsp::SmallVector<std::shared_ptr<int>, 4> vec {
            std::make_shared<int> (1),
            std::make_shared<int> (2),
            std::make_shared<int> (3),
        };
        vec.resize (3);
        REQUIRE (*vec[2] == 3);

        vec.clear();
        REQUIRE (vec.empty());

        vec.emplace_back (std::make_shared<int> (0));
        vec.emplace_back (std::make_shared<int> (1));
        vec.emplace_back (std::make_shared<int> (2));
        vec.emplace_back (std::make_shared<int> (3));
        vec.emplace_back (std::make_shared<int> (4));
        REQUIRE (vec.size() == 5);

        vec.clear();
        REQUIRE (vec.empty());
    }

    SECTION ("Insert/Emplace Single Value Test")
    {
        chowdsp::SmallVector<double, 4> vec { 1.0, 2.0 };

        vec.insert (vec.begin() + 1, 1.5);
        REQUIRE (vec.size() == 3);
        REQUIRE (juce::exactlyEqual (vec[1], 1.5));

        vec.insert (vec.begin() + 1, 1.25);
        REQUIRE (vec.size() == 4);
        REQUIRE (juce::exactlyEqual (vec[1], 1.25));

        vec.insert (vec.begin() + 3, 1.75);
        REQUIRE (vec.size() == 5);
        REQUIRE (juce::exactlyEqual (vec[3], 1.75));
    }

    SECTION ("Insert Copy Single Value Test")
    {
        std::string _1_5 = "1.5";
        std::string _1_25 = "1.25";
        std::string _1_75 = "1.75";
        chowdsp::SmallVector<std::string, 4> vec { "1.0", "2.0" };

        vec.insert (vec.begin() + 1, _1_5);
        REQUIRE (vec.size() == 3);
        REQUIRE (vec[1] == _1_5);

        vec.insert (vec.begin() + 1, _1_25);
        REQUIRE (vec.size() == 4);
        REQUIRE (vec[1] == _1_25);

        vec.insert (vec.begin() + 3, _1_75);
        REQUIRE (vec.size() == 5);
        REQUIRE (vec[3] == _1_75);
    }

    SECTION ("Insert Range Test")
    {
        std::array<double, 4> decimals { 1.2, 1.4, 1.6, 1.8 };
        {
            chowdsp::SmallVector<double, 6> vec { 1.0, 2.0 };
            vec.insert (vec.begin() + 1, decimals.begin(), decimals.end());
            REQUIRE (vec.size() == 6);
            REQUIRE (juce::exactlyEqual (vec[1], 1.2));
            REQUIRE (juce::exactlyEqual (vec[3], 1.6));
            REQUIRE (juce::exactlyEqual (vec[5], 2.0));
        }
        {
            chowdsp::SmallVector<double, 8> vec { 1.0, 2.0 };
            vec.insert (vec.begin() + 1, decimals.begin(), decimals.end());
            REQUIRE (vec.size() == 6);
            REQUIRE (juce::exactlyEqual (vec[1], 1.2));
            REQUIRE (juce::exactlyEqual (vec[3], 1.6));
            REQUIRE (juce::exactlyEqual (vec[5], 2.0));
        }
        {
            chowdsp::SmallVector<double, 2> vec { 1.0, 2.0 };
            vec.insert (vec.begin() + 1, { 1.2, 1.4, 1.6, 1.8 });
            REQUIRE (vec.size() == 6);
            REQUIRE (juce::exactlyEqual (vec[1], 1.2));
            REQUIRE (juce::exactlyEqual (vec[3], 1.6));
            REQUIRE (juce::exactlyEqual (vec[5], 2.0));
        }
    }

    SECTION ("Insert Multiple of Same Test")
    {
        {
            chowdsp::SmallVector<double, 2> vec { 1.0, 2.0 };
            vec.insert (vec.begin() + 1, 3, 1.5);
            REQUIRE (vec.size() == 5);
            REQUIRE (juce::exactlyEqual (vec[1], 1.5));
            REQUIRE (juce::exactlyEqual (vec[3], 1.5));
            REQUIRE (juce::exactlyEqual (vec[4], 2.0));
        }
        {
            chowdsp::SmallVector<double, 8> vec { 1.0, 2.0 };
            vec.insert (vec.begin() + 1, 3, 1.5);
            REQUIRE (vec.size() == 5);
            REQUIRE (juce::exactlyEqual (vec[1], 1.5));
            REQUIRE (juce::exactlyEqual (vec[3], 1.5));
            REQUIRE (juce::exactlyEqual (vec[4], 2.0));
        }
    }

    SECTION ("Erase Test")
    {
        {
            chowdsp::SmallVector<int, 2> vec { 1, 2, 3, 4, 5 };

            vec.erase (vec.begin() + 2);
            REQUIRE (vec.size() == 4);
            REQUIRE (vec[0] == 1);
            REQUIRE (vec[1] == 2);
            REQUIRE (vec[2] == 4);
            REQUIRE (vec[3] == 5);

            const auto shouldBeEnd = vec.erase (vec.begin() + 3);
            REQUIRE (vec.end() == shouldBeEnd);
            REQUIRE (vec.size() == 3);
            REQUIRE (vec[0] == 1);
            REQUIRE (vec[1] == 2);
            REQUIRE (vec[2] == 4);
        }

        {
            chowdsp::SmallVector<int, 5> vec { 1, 2, 3, 4, 5 };

            vec.erase (vec.begin() + 2);
            REQUIRE (vec.size() == 4);
            REQUIRE (vec[0] == 1);
            REQUIRE (vec[1] == 2);
            REQUIRE (vec[2] == 4);
            REQUIRE (vec[3] == 5);

            const auto shouldBeEnd = vec.erase (vec.begin() + 3);
            REQUIRE (vec.end() == shouldBeEnd);
            REQUIRE (vec.size() == 3);
            REQUIRE (vec[0] == 1);
            REQUIRE (vec[1] == 2);
            REQUIRE (vec[2] == 4);
        }
    }

    SECTION ("Erase Range Test")
    {
        {
            chowdsp::SmallVector<int, 2> vec { 1, 2, 3, 4, 5 };

            vec.erase (vec.begin() + 1, vec.begin() + 3);
            REQUIRE (vec.size() == 3);
            REQUIRE (vec[0] == 1);
            REQUIRE (vec[1] == 4);
            REQUIRE (vec[2] == 5);

            const auto shouldBeEnd = vec.erase (vec.begin() + 2, vec.end());
            REQUIRE (vec.end() == shouldBeEnd);
            REQUIRE (vec.size() == 2);
            REQUIRE (vec[0] == 1);
            REQUIRE (vec[1] == 4);
        }

        {
            chowdsp::SmallVector<int, 5> vec { 1, 2, 3, 4, 5 };

            vec.erase (vec.begin() + 1, vec.begin() + 3);
            REQUIRE (vec.size() == 3);
            REQUIRE (vec[0] == 1);
            ;
            REQUIRE (vec[1] == 4);
            REQUIRE (vec[2] == 5);

            const auto shouldBeEnd = vec.erase (vec.begin() + 2, vec.end());
            REQUIRE (vec.end() == shouldBeEnd);
            REQUIRE (vec.size() == 2);
            REQUIRE (vec[0] == 1);
            REQUIRE (vec[1] == 4);
        }
    }
}
