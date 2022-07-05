#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>
#include <array>

using namespace chowdsp::MemoryUtils;

TEST_CASE ("Memory Aliasing Test")
{
    SECTION ("Raw Memory Test")
    {
        int x[10];
        REQUIRE (doesMemoryAlias (x, 10, x, 5));
        REQUIRE (doesMemoryAlias (x, 10, x + 5, 5));
        REQUIRE (! doesMemoryAlias (x, 5, x + 5, 5));
    }

    SECTION ("C-Style Array Test")
    {
        int x[10];
        int y[20];
        REQUIRE (doesMemoryAlias (x, x));
        REQUIRE (! doesMemoryAlias (x, y));
    }

    SECTION ("std::array Test")
    {
        std::array<int, 10> x {};
        std::array<int, 20> y {};
        REQUIRE (doesMemoryAlias (x, x));
        REQUIRE (! doesMemoryAlias (x, y));
    }

    SECTION ("std::vector Test")
    {
        std::vector<int> x { 0, 1, 2, 3 };
        std::vector<int> y { 0, 1, 2 };
        REQUIRE (doesMemoryAlias (x, x));
        REQUIRE (! doesMemoryAlias (x, y));
    }

    SECTION ("Iterators Test")
    {
        std::vector<int> x { 0, 1, 2, 3 };
        std::vector<int> y { 0, 1, 2 };
        REQUIRE (doesMemoryAlias (x.begin(), x.end(), x.begin(), x.end()));
        REQUIRE (! doesMemoryAlias (x.begin(), x.end(), y.begin(), y.end()));
        REQUIRE (doesMemoryAlias (x.begin(), x.begin() + 2, x.begin(), x.end()));
        REQUIRE (! doesMemoryAlias (x.begin(), x.begin() + 2, x.begin() + 2, x.end()));
    }
}
