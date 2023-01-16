#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>
#include <array>

using namespace chowdsp::MemoryUtils;

TEST_CASE ("Memory Aliasing Test", "[common][memory]")
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
        int x[10] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        int y[20] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };

        INFO ("Don't optimize away... " << std::to_string (x[5]) << " -- " << std::to_string (y[10]));
        REQUIRE (doesMemoryAlias (x, x));
        REQUIRE (! doesMemoryAlias (x, y));
    }

    SECTION ("std::array Test")
    {
        std::array<int, 10> x { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        std::array<int, 20> y { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
        INFO ("Don't optimize away... " << std::to_string (x[5]) << " -- " << std::to_string (y[10]));
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
