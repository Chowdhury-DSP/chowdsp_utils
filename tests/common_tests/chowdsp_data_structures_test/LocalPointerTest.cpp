#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Local Pointer Test", "[common][data-structures]")
{
    SECTION ("Forward Declaration")
    {
        struct Test;
        chowdsp::LocalPointer<Test, 8> ptr;
        REQUIRE (ptr == nullptr);

        struct Test
        {
            Test (float x, float y) : data ({ x, y }) {}
            std::array<float, 2> data {};
        };
        ptr.emplace (5.0f, 6.0f);
        REQUIRE (juce::exactlyEqual (ptr->data[0], 5.0f));
        REQUIRE (juce::exactlyEqual (ptr->data[1], 6.0f));

        ptr->data[0] = 7.0f;
        REQUIRE (juce::exactlyEqual (std::as_const (ptr)->data[0], 7.0f));

        (*ptr).data[1] = 8.0f;
        REQUIRE (juce::exactlyEqual ((*std::as_const (ptr)).data[1], 8.0f));

        ptr.reset();
        REQUIRE (ptr == nullptr);
    }

    SECTION ("Type with active destructor")
    {
        struct Test;
        chowdsp::LocalPointer<Test, 16> ptr;

        struct Object
        {
            JUCE_LEAK_DETECTOR (Object)
        };

        struct Test
        {
            explicit Test (int& x) : thing (x)
            {
                object = std::make_unique<Object>();
            }
            ~Test()
            {
                thing = -1001;
            }
            int& thing;
            std::unique_ptr<Object> object;
        };

        int xx = 44;
        ptr.emplace (xx);
        REQUIRE (xx == 44);

        ptr.reset();
        REQUIRE (xx == -1001);
    }

    SECTION ("Abstract Type")
    {
        struct Base;
        chowdsp::LocalPointer<Base, 8> ptr;

        struct Base
        {
            explicit Base (int x) : xx (x) {}
            int xx = 44;
        };
        ptr.emplace (12);
        REQUIRE (ptr->xx == 12);

        struct Child : Base
        {
            Child (int x, int y) : Base (x), yy (y) {}
            int yy = 1001;
        };

        auto* childPtr = ptr.emplace<Child> (45, 46);
        REQUIRE (ptr.get() == childPtr);
        REQUIRE (ptr->xx == 45);
        REQUIRE (childPtr->yy == 46);
    }

    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324 4319)
    static constexpr size_t testAlignment = 32;
    SECTION ("Alignment Known")
    {
        struct alignas (testAlignment) Test
        {
        };
        chowdsp::LocalPointer<Test, 64> ptr;
        ptr.emplace();
        const auto alignedPtr = juce::snapPointerToAlignment (ptr.get(), testAlignment);
        REQUIRE (alignedPtr == ptr.get());
    }

    SECTION ("Alignment Forward Declared")
    {
        chowdsp::LocalPointer<struct Test, 64, testAlignment> ptr;
        struct alignas (testAlignment) Test
        {
        };
        ptr.emplace();
        const auto alignedPtr = juce::snapPointerToAlignment (ptr.get(), testAlignment);
        REQUIRE (alignedPtr == ptr.get());
    }
    JUCE_END_IGNORE_WARNINGS_MSVC
}
