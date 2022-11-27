#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("Smart Pointer Test")
{
    struct TestType
    {
        TestType (int _x, int _y) : x (_x), y (_y) {}
        int x;
        int y;
    };

    static_assert (chowdsp::IsOptionalPointer<chowdsp::OptionalPointer<TestType>>);
    static_assert (! chowdsp::IsOptionalPointer<TestType>);

    SECTION ("Owning")
    {
        chowdsp::OptionalPointer<TestType> x { 4, 5 };
        REQUIRE (x.isOwner());
        REQUIRE (x->x == 4);
        REQUIRE (x->y == 5);

        x.invalidate();
        REQUIRE (x == nullptr);
    }

    SECTION ("Non-Owning")
    {
        const auto x_owned = std::make_unique<TestType> (4, 5);
        const chowdsp::OptionalPointer<TestType> x { x_owned.get() };
        REQUIRE (! x.isOwner());
        REQUIRE (x->x == 4);
        REQUIRE (x->y == 5);
    }

    SECTION ("Owning -> Non-Owning")
    {
        chowdsp::OptionalPointer<TestType> x { 4, 5 };
        REQUIRE (x.isOwner());
        REQUIRE (x->x == 4);
        REQUIRE (x->y == 5);

        {
            std::unique_ptr<TestType> x_owned { x.release() };
            x_owned->x = 0;
            x_owned->y = 0;
            REQUIRE (! x.isOwner());
            REQUIRE ((*x).x == 0);
            REQUIRE ((*x).y == 0);

            x.invalidate();
        }

        REQUIRE (x == nullptr);
    }

    SECTION ("Equality with nullptr")
    {
        const chowdsp::OptionalPointer<TestType> x { 4, 5 };
        REQUIRE (x != nullptr);
        REQUIRE_FALSE (x == nullptr);
        REQUIRE (nullptr != x);
        REQUIRE_FALSE (nullptr == x);
    }

    SECTION ("Equality with unique_ptr")
    {
        chowdsp::OptionalPointer<TestType> x { 4, 5 };
        std::unique_ptr<TestType> x_owned { x.release() };
        REQUIRE (x == x_owned);
        REQUIRE_FALSE (x != x_owned);
        REQUIRE (x_owned == x);
        REQUIRE_FALSE (x_owned != x);
    }

    SECTION ("Equality with other OptionalPointer")
    {
        chowdsp::OptionalPointer<TestType> x { 4, 5 };
        chowdsp::OptionalPointer<TestType> y { x.get() };
        REQUIRE (x == y);
        REQUIRE_FALSE (x != y);
    }
}
