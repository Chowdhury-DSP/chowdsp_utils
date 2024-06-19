#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("OptionalRef Test", "[common][data-structures]")
{
    int x = 42;
    chowdsp::OptionalRef x_opt { x };

    SECTION ("has_value()")
    {
        REQUIRE (x_opt.has_value());
        REQUIRE (x_opt.has_value());
    }

    SECTION ("operator->()")
    {
        REQUIRE (x_opt.operator->() == &x);
        REQUIRE (std::as_const (x_opt).operator->() == &x);
    }

    SECTION ("value()")
    {
        REQUIRE (std::as_const (x_opt).value() == 42);
        x_opt.value() = 43;
        REQUIRE (std::as_const (x_opt).value() == 43);
        *x_opt = 44;
        REQUIRE (*std::as_const (x_opt) == 44);
    }

    SECTION ("value() throws")
    {
        chowdsp::OptionalRef<int> opt {};
        REQUIRE_THROWS (opt.value() == 0);
        REQUIRE_THROWS (std::as_const (opt).value() == 0);
    }

    SECTION ("swap()")
    {
        chowdsp::OptionalRef<int> x_other {};
        x_opt.swap (x_other);
        REQUIRE (! x_opt.has_value());
        REQUIRE (x_other.has_value());
        x_opt.swap (x_other);
        REQUIRE (x_opt.has_value());
        REQUIRE (! x_other.has_value());
    }

    SECTION ("reset()")
    {
        x_opt.reset();
        REQUIRE (! x_opt);
    }

    SECTION ("assignment")
    {
        int y = 100;
        x_opt = y;
        REQUIRE (*x_opt == 100);
    }

    SECTION ("nullopt")
    {
        x_opt = std::nullopt;
        REQUIRE (! x_opt);
        chowdsp::OptionalRef<int> y_opt { std::nullopt };
        REQUIRE (! y_opt);
    }

    SECTION ("value_or()")
    {
        REQUIRE (x_opt.value_or (-10) == 42);
        x_opt = std::nullopt;
        REQUIRE (x_opt.value_or (-10) == -10);
    }
}
