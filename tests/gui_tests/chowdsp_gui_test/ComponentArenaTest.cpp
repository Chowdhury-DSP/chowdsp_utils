#include <CatchUtils.h>
#include <chowdsp_gui/chowdsp_gui.h>

TEST_CASE ("Component Arena Test", "[gui][data-structures]")
{
    struct TestComponent : juce::Component
    {
        int& destructor_counter;
        size_t other_arg {};
        explicit TestComponent (int& dest_counter) : destructor_counter (dest_counter) {}
        TestComponent (int& dest_counter, size_t other) : destructor_counter (dest_counter), other_arg (other) {}
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
        REQUIRE (arena.component_count() == 5);

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
        REQUIRE (arena.component_count() == 5);

        REQUIRE (destructor_counter == 0);
        arena.clear_all();
        REQUIRE (destructor_counter == 5);
    }

    SECTION ("allocate_n with args")
    {
        arena.allocate<TestComponent> (destructor_counter);
        const auto array = arena.allocate_n_lambda<TestComponent> (4,
                                                                   [&destructor_counter] (size_t idx)
                                                                   {
                                                                       return TestComponent { destructor_counter, idx * 2 + 1 };
                                                                   });
        REQUIRE (arena.component_count() == 5);

        for (auto [idx, component] : chowdsp::enumerate (array))
            REQUIRE (component.other_arg == 2 * idx + 1);

        REQUIRE (destructor_counter == 0);
        arena.clear_all();
        REQUIRE (destructor_counter == 5);
    }
}
