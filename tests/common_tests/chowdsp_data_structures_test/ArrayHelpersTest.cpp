#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Array Helpers Test", "[common][data-structures]")
{
    struct NeedsArgs
    {
        constexpr NeedsArgs (int x_val, int y_val)
            : x { x_val },
              y { y_val }
        {
        }

        //        NeedsArgs (const NeedsArgs&) = delete;
        //        NeedsArgs& operator= (const NeedsArgs&) = delete;
        //        NeedsArgs (NeedsArgs&&) noexcept = delete;
        //        NeedsArgs& operator= (NeedsArgs&&) noexcept = delete;

        const int x;
        const int y;
    };

    SECTION ("Make Array")
    {
        static constexpr auto arr = chowdsp::make_array<NeedsArgs, 2> (100, 101);
        STATIC_REQUIRE (arr.size() == 2);
        STATIC_REQUIRE (arr[0].x == 100);
        STATIC_REQUIRE (arr[0].y == 101);
        STATIC_REQUIRE (arr[1].x == 100);
        STATIC_REQUIRE (arr[1].y == 101);
    }

    SECTION ("Make Array with Lambda")
    {
        static constexpr auto arr = chowdsp::make_array_lambda<NeedsArgs, 2> (
            [] (auto index)
            {
                return NeedsArgs {
                    2 * (int) index,
                    2 * (int) index + 1,
                };
            });
        STATIC_REQUIRE (arr.size() == 2);
        STATIC_REQUIRE (arr[0].x == 0);
        STATIC_REQUIRE (arr[0].y == 1);
        STATIC_REQUIRE (arr[1].x == 2);
        STATIC_REQUIRE (arr[1].y == 3);
    }
}