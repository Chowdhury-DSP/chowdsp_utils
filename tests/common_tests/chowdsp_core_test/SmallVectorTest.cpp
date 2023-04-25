#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("Small Vector Test", "[common][data-structures]")
{
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

    SECTION ("Copy construction")
    {
        chowdsp::SmallVector<std::string, 4> vec { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 4> vec2 { vec };
        REQUIRE (vec.size() == vec2.size());
        for (const auto [ref, actual] : chowdsp::zip (vec, vec2))
            REQUIRE (ref == actual);
    }

    SECTION ("Copy assign")
    {
        chowdsp::SmallVector<std::string, 2> vec { "john", "paul", "george", "ringo" };
        const auto vec2 = vec.operator= (vec);
        REQUIRE (vec.size() == vec2.size());
        for (const auto [ref, actual] : chowdsp::zip (vec, vec2))
            REQUIRE (ref == actual);
    }

    SECTION ("Move construction")
    {
        std::array<std::string, 4> beatles { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 4> vec { beatles.begin(), beatles.end() };
        chowdsp::SmallVector<std::string, 4> vec2 { std::move (vec) };
        REQUIRE (beatles.size() == vec2.size());
        for (const auto [ref, actual] : chowdsp::zip (beatles, vec2))
            REQUIRE (ref == actual);
    }

    SECTION ("Copy assign")
    {
        std::array<std::string, 4> beatles { "john", "paul", "george", "ringo" };
        chowdsp::SmallVector<std::string, 2> vec { beatles.begin(), beatles.end() };
        const auto vec2 = std::move (vec);
        REQUIRE (beatles.size() == vec2.size());
        for (const auto [ref, actual] : chowdsp::zip (beatles, vec2))
            REQUIRE (ref == actual);
    }

    SECTION ("Front/Back/Data (small)")
    {
        chowdsp::SmallVector<std::string, 4> vec { "john", "paul", "george", "ringo" };
        REQUIRE (vec.front() == "john");
        REQUIRE (std::as_const (vec).front() == "john");
        REQUIRE (vec.back() == "ringo");
        REQUIRE (std::as_const (vec).back() == "ringo");
        REQUIRE (*vec.data() == "john");
        REQUIRE (*std::as_const (vec).data() == "john");
    }

    SECTION ("Front/Back/Data (large)")
    {
        chowdsp::SmallVector<std::string, 2> vec { "john", "paul", "george", "ringo" };
        REQUIRE (vec.front() == "john");
        REQUIRE (std::as_const (vec).front() == "john");
        REQUIRE (vec.back() == "ringo");
        REQUIRE (std::as_const (vec).back() == "ringo");
        REQUIRE (*vec.data() == "john");
        REQUIRE (*std::as_const (vec).data() == "john");
    }

    SECTION ("Reserve")
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

    SECTION ("Clear")
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
}
