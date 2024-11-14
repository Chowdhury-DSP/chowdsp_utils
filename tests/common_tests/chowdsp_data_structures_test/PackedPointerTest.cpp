#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Packed Pointer Test", "[common][data-structures]")
{
    struct alignas (8) TestType
    {
        TestType (int _x, int _y) : x (_x), y (_y) {}
        int x;
        int y;
    };

    TestType t1 { 4, 5 };
    TestType t2 { 6, 7 };

    SECTION ("Null Init")
    {
        chowdsp::PackedPointer<TestType> ptr {};
        REQUIRE (ptr == nullptr);
        REQUIRE (nullptr == ptr);
        REQUIRE (ptr != &t2);
        REQUIRE (&t2 != ptr);
    }

    SECTION ("Init No Flags")
    {
        chowdsp::PackedPointer<TestType> ptr { &t1 };
        REQUIRE (ptr == &t1);
        REQUIRE (ptr != nullptr);
        REQUIRE (ptr.get_flags() == 0);
    }

    SECTION ("Flags")
    {
        chowdsp::PackedPointer<TestType> ptr { &t1, 5 };
        REQUIRE (ptr == &t1);
        REQUIRE (nullptr != ptr);
        REQUIRE (ptr.get_flags() == 5);
        REQUIRE ((*ptr).x == t1.x);

        ptr.set_flags (7);
        REQUIRE (&t1 == ptr);
        REQUIRE (ptr.get_flags() == 7);
        REQUIRE ((*std::as_const (ptr)).y == t1.y);
    }

    SECTION ("Modify")
    {
        chowdsp::PackedPointer ptr { &t1, 5 };
        ptr->x = 100;
        REQUIRE (std::as_const (ptr)->x == t1.x);
    }

    SECTION ("Swap")
    {
        chowdsp::PackedPointer ptr { &t1, 5 };
        chowdsp::PackedPointer<TestType> ptr2 {};
        ptr.swap (ptr2);
        REQUIRE (ptr == nullptr);
        REQUIRE (std::as_const (ptr2)->x == t1.x);
    }
}
