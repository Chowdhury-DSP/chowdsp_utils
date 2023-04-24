#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

template <typename T>
class reverse
{
private:
    T& iterable_;

public:
    explicit reverse (T& iterable) : iterable_ { iterable } {}
    auto begin() const { return std::rbegin (iterable_); }
    auto end() const { return std::rend (iterable_); }
};

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
                for (auto& v : reverse (vec))
                    v = count++;
            }

            for (const auto [n, v] : chowdsp::enumerate (reverse (std::as_const (vec))))
                REQUIRE ((int) n == v);
        }
    }

    // TODO:
    // - resize() with same size
    // - clear()
    // - reserve()
    // - front(), back(), data()
    // - moving, copying, and other contructors
}
