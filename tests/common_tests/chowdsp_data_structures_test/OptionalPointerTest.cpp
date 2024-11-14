#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Optional Pointer Test", "[common][data-structures]")
{
    struct alignas (8) TestType
    {
        TestType (int _x, int _y) : x (_x), y (_y) {}
        int x;
        int y;
    };

    static_assert (chowdsp::IsOptionalPointer<chowdsp::OptionalPointer<TestType>>);
    static_assert (! chowdsp::IsOptionalPointer<TestType>);

    SECTION ("Owning")
    {
        auto x = chowdsp::OptionalPointer<TestType> (4, 5);
        REQUIRE (x.isOwner());
        REQUIRE (x->x == 4);
        REQUIRE (x->y == 5);

        x.invalidate();
        REQUIRE (x == nullptr);
    }

    SECTION ("Non-Owning")
    {
        const auto x_owned = std::make_unique<TestType> (4, 5);
        auto x = chowdsp::OptionalPointer<TestType> (x_owned.get(), false);
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

    SECTION ("Non-Owning -> Owning")
    {
        auto x_owned = std::make_unique<TestType> (4, 5);
        chowdsp::OptionalPointer<TestType> x;
        x.setNonOwning (x_owned.get());
        REQUIRE (! x.isOwner());
        REQUIRE (x->x == 4);
        REQUIRE (x->y == 5);

        x.setOwning (nullptr);
        REQUIRE (x == nullptr);

        x.setOwning (std::move (x_owned));
        REQUIRE (x.isOwner());
        REQUIRE (x->x == 4);
        REQUIRE (x->y == 5);

        x.setOwning (nullptr);
        REQUIRE (x == nullptr);
    }

    SECTION ("From Raw Pointer (Owning)")
    {
        auto x = chowdsp::OptionalPointer<TestType> (4, 5);
        REQUIRE (x.isOwner());
        REQUIRE (x->x == 4);
        REQUIRE (x->y == 5);

        auto y = chowdsp::OptionalPointer<TestType> (x.release());
        REQUIRE (! x.isOwner());
        REQUIRE (y.isOwner());
        REQUIRE (y->x == 4);
        REQUIRE (y->y == 5);
    }

    SECTION ("From Raw Pointer (Non-Owning)")
    {
        auto x = chowdsp::OptionalPointer<TestType> (4, 5);
        REQUIRE (x.isOwner());
        REQUIRE (x->x == 4);
        REQUIRE (x->y == 5);

        auto y = chowdsp::OptionalPointer<TestType> (x.get(), false);
        REQUIRE (x.isOwner());
        REQUIRE (! y.isOwner());
        REQUIRE (y->x == 4);
        REQUIRE (y->y == 5);
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
        chowdsp::OptionalPointer<TestType> y;
        y.setNonOwning (x.get());
        REQUIRE (x == y);
        REQUIRE_FALSE (x != y);
    }

    SECTION ("Move Constructor")
    {
        chowdsp::OptionalPointer<TestType> x { 4, 5 };
        chowdsp::OptionalPointer<TestType> y { std::move (x) };
        REQUIRE (x == nullptr); // NOLINT
        REQUIRE (y->x == 4);
        REQUIRE (y->y == 5);
    }

    SECTION ("Move Assignment")
    {
        chowdsp::OptionalPointer<TestType> y {};
        y = chowdsp::OptionalPointer<TestType> { 4, 5 };
        REQUIRE (y->x == 4);
        REQUIRE (y->y == 5);
    }
}
