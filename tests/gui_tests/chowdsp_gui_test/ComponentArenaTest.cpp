#include <CatchUtils.h>
#include <chowdsp_gui/chowdsp_gui.h>

TEST_CASE ("Component Arena Test", "[gui][data-structures]")
{
    struct TestComponent : juce::Component
    {
        int& destructor_counter;
        explicit TestComponent (int& dest_counter) : destructor_counter (dest_counter) {}
        ~TestComponent() override { ++destructor_counter; }
    };

    struct NotAComponent
    {
        int& destructor_counter;
        explicit NotAComponent (int& dest_counter) : destructor_counter (dest_counter) {}
        ~NotAComponent() { ++destructor_counter; }
    };

    int destructor_counter = 0;
    chowdsp::ComponentArena<> arena {};

    SECTION ("Only allocate components")
    {
        arena.allocate<TestComponent> (destructor_counter);
        arena.allocate_n<TestComponent> (4, destructor_counter);

        REQUIRE (destructor_counter == 0);
        arena.clear_all();
        REQUIRE (destructor_counter == 5);
    }

    SECTION ("Allocate other types too")
    {
        arena.allocate<TestComponent> (destructor_counter);
        arena.allocate<NotAComponent> (destructor_counter);
        arena.allocate_n<TestComponent> (4, destructor_counter);
        arena.allocate_n<NotAComponent> (3, destructor_counter);

        REQUIRE (destructor_counter == 0);
        arena.clear_all();
        REQUIRE (destructor_counter == 5);
    }
}
