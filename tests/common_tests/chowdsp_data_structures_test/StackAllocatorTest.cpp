#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Stack Allocator Test", "[common][data-structures]")
{
    chowdsp::StackAllocator allocator;
    allocator.reset (150);

    // allocate doubles
    {
        auto* some_doubles = allocator.allocate<double> (10);
        REQUIRE ((void*) some_doubles == allocator.data<void> (0));
        REQUIRE (allocator.get_bytes_used() == 80);
    }

    // allocate ints
    {
        auto* some_ints = allocator.allocate<int32_t> (10);
        REQUIRE ((void*) some_ints == allocator.data<void> (80));
        REQUIRE (allocator.get_bytes_used() == 120);
    }

    // allocate with stack frame
    {
        chowdsp::StackAllocator::StackAllocatorFrame frame { allocator };
        auto* some_chars = allocator.allocate<char> (30);
        juce::ignoreUnused (some_chars);
        REQUIRE (allocator.get_bytes_used() == 150);
    }

    REQUIRE (allocator.get_bytes_used() == 120);

    // aligned allocation
    {
        auto* some_data = allocator.allocate<float> (1, 16);
        REQUIRE (juce::snapPointerToAlignment (some_data, 16) == some_data);
    }

    // overfull allocation
    REQUIRE (allocator.allocate<double> (100) == nullptr);

    // clear allocator
    allocator.clear();
    REQUIRE (allocator.get_bytes_used() == 0);
}
