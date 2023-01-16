#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("Bindings Test", "[common][functional]")
{
    using namespace chowdsp::Functional;

    struct Point
    {
        int x = 0, y = 0;

        void displace (int x_off, int y_off)
        {
            x += x_off;
            y += y_off;
        }
    };

    SECTION ("Bind Front Test")
    {
        const auto sum_func = [] (int a, int b, int c, int d)
        {
            return a + b + c + d;
        };
        const auto sum_two = bind_front (sum_func, 1, 2);
        REQUIRE (sum_two (3, 4) == 10);
    }

    SECTION ("Bind Front Struct Test")
    {
        Point p {};
        auto displace = bind_front (&Point::displace, &p);

        displace (3, 4);

        REQUIRE (p.x == 3);
        REQUIRE (p.y == 4);
    }

    SECTION ("Bind Back Test")
    {
        const auto func = [] (int a, int b, int c, int d)
        {
            return a + b - c - d;
        };
        const auto sum_two = bind_back (func, 1, 2);
        REQUIRE (sum_two (3, 4) == 4);
    }

    SECTION ("Bind Back Struct Test")
    {
        auto displace_3_4 = bind_back (&Point::displace, 3, 4);

        Point p {};
        displace_3_4 (&p);

        REQUIRE (p.x == 3);
        REQUIRE (p.y == 4);
    }
}
