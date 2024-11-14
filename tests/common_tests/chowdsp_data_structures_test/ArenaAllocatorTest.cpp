#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Arena Allocator Test", "[common][data-structures]")
{
    SECTION ("With std::vector<std::byte>")
    {
        chowdsp::ArenaAllocator allocator { 150 };

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
            const auto frame = allocator.create_frame();
            auto* some_chars = allocator.allocate<char> (30);
            juce::ignoreUnused (some_chars);
            REQUIRE (allocator.get_bytes_used() == 150);
        }

        REQUIRE (allocator.get_bytes_used() == 120);

        // aligned allocation
        {
            auto* some_data = allocator.allocate<float> (1, 16);
            REQUIRE (juce::snapPointerToAlignment (some_data, (size_t) 16) == some_data);
        }

        // overfull allocation
        REQUIRE (allocator.allocate<double> (100) == nullptr);

        // clear allocator
        allocator.clear();
        REQUIRE (allocator.get_bytes_used() == 0);
    }

    SECTION ("With std::array<std::byte>")
    {
        struct ArrayMemoryResource : std::array<std::byte, 150>
        {
            void resize (size_t N, std::byte value = {}) { juce::ignoreUnused (N, value); } // NOLINT
        };

        chowdsp::ArenaAllocator<ArrayMemoryResource> allocator { 150 };

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
            const auto frame = allocator.create_frame();
            auto* some_chars = allocator.allocate<char> (30);
            juce::ignoreUnused (some_chars);
            REQUIRE (allocator.get_bytes_used() == 150);
        }

        REQUIRE (allocator.get_bytes_used() == 120);

        // aligned allocation
        {
            auto* some_data = allocator.allocate<float> (1, 16);
            REQUIRE (juce::snapPointerToAlignment (some_data, (size_t) 16) == some_data);
        }

        // overfull allocation
        REQUIRE (allocator.allocate<double> (100) == nullptr);

        // clear allocator
        allocator.clear();
        REQUIRE (allocator.get_bytes_used() == 0);
    }
}
