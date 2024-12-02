#include <CatchUtils.h>
#include <chowdsp_serialization/chowdsp_serialization.h>

TEST_CASE ("Byte Serialization Test", "[common][serialization]")
{
    struct Test
    {
        int x = 44;
        float y = 128.0f;
        double z = -19.0f;
        bool b = false;
    };

    SECTION ("Basic Arena")
    {
        chowdsp::ArenaAllocator<std::array<std::byte, 1024>> arena {};

        const auto arr = chowdsp::make_array_lambda<float, 12> ([] (auto idx)
                                                                { return static_cast<float> (idx); });
        const auto str = std::string { "Hello world" };

        chowdsp::serialize_object (int { 42 }, arena);
        chowdsp::serialize_object (float { 99.0f }, arena);
        chowdsp::serialize_object (Test {}, arena);
        chowdsp::serialize_object (Test { 1, 2.0f, 3.0, true }, arena);
        chowdsp::serialize_span<float> (arr, arena);
        chowdsp::serialize_string (str, arena);

        auto bytes = chowdsp::dump_serialized_bytes (arena);

        const auto int_test = chowdsp::deserialize_object<int> (bytes);
        REQUIRE (int_test == 42);
        const auto float_test = chowdsp::deserialize_object<float> (bytes);
        REQUIRE (float_test == 99.0f);
        auto struct_test = chowdsp::deserialize_object<Test> (bytes);
        REQUIRE (pfr::eq_fields (struct_test, Test {}));
        struct_test = chowdsp::deserialize_object<Test> (bytes);
        REQUIRE (pfr::eq_fields (struct_test, Test { 1, 2.0f, 3.0, true }));

        const auto arr_test = chowdsp::deserialize_object<std::array<float, 12>> (bytes);
        for (const auto& [x, y] : chowdsp::zip (arr, arr_test))
            REQUIRE (juce::exactlyEqual (x, y));

        const auto str_test = chowdsp::deserialize_string (bytes);
        REQUIRE (str_test == str);
        REQUIRE (bytes.empty());
    }

    SECTION ("Arena w/ previous usage")
    {
        chowdsp::ArenaAllocator<std::array<std::byte, 1024>> arena {};
        arena.allocate<float> (30);

        std::vector<float> vec (12);
        std::iota (vec.begin(), vec.end(), 100);
        const auto str = juce::String { "Hello world" };

        const auto frame = arena.create_frame();
        chowdsp::serialize_object (int { 42 }, arena);
        chowdsp::serialize_object (float { 99.0f }, arena);
        chowdsp::serialize_object (Test {}, arena);
        chowdsp::serialize_object (Test { 1, 2.0f, 3.0, true }, arena);
        chowdsp::serialize_span<float> (vec, arena);
        chowdsp::serialize_string (chowdsp::toStringView (str), arena);

        auto bytes = chowdsp::dump_serialized_bytes (arena, &frame);

        const auto int_test = chowdsp::deserialize_object<int> (bytes);
        REQUIRE (int_test == 42);
        const auto float_test = chowdsp::deserialize_object<float> (bytes);
        REQUIRE (float_test == 99.0f);
        auto struct_test = chowdsp::deserialize_object<Test> (bytes);
        REQUIRE (pfr::eq_fields (struct_test, Test {}));
        struct_test = chowdsp::deserialize_object<Test> (bytes);
        REQUIRE (pfr::eq_fields (struct_test, Test { 1, 2.0f, 3.0, true }));

        std::vector<float> vec_test (chowdsp::get_span_length<float> (bytes));
        REQUIRE (vec_test.size() == vec.size());
        chowdsp::deserialize_span<float> (vec_test, bytes);
        for (const auto& [x, y] : chowdsp::zip (vec, vec_test))
            REQUIRE (juce::exactlyEqual (x, y));

        const auto str_test = chowdsp::deserialize_string (bytes);
        REQUIRE (str_test == chowdsp::toStringView (str));
        REQUIRE (bytes.empty());
    }

    SECTION ("Basic Chained Arena")
    {
        chowdsp::ChainedArenaAllocator arena { 64 };

        const auto arr = chowdsp::make_array_lambda<float, 12> ([] (auto idx)
                                                                { return static_cast<float> (idx); });
        const auto str = std::string { "Hello world" };

        chowdsp::serialize_object (int { 42 }, arena);
        chowdsp::serialize_object (float { 99.0f }, arena);
        chowdsp::serialize_object (Test {}, arena);
        chowdsp::serialize_object (Test { 1, 2.0f, 3.0, true }, arena);
        chowdsp::serialize_span<float> (arr, arena);
        chowdsp::serialize_string (str, arena);

        std::vector<std::byte> raw_bytes (chowdsp::get_serial_num_bytes (arena));
        chowdsp::dump_serialized_bytes (raw_bytes, arena);
        nonstd::span<const std::byte> bytes { raw_bytes };

        const auto int_test = chowdsp::deserialize_object<int> (bytes);
        REQUIRE (int_test == 42);
        const auto float_test = chowdsp::deserialize_object<float> (bytes);
        REQUIRE (float_test == 99.0f);
        auto struct_test = chowdsp::deserialize_object<Test> (bytes);
        REQUIRE (pfr::eq_fields (struct_test, Test {}));
        struct_test = chowdsp::deserialize_object<Test> (bytes);
        REQUIRE (pfr::eq_fields (struct_test, Test { 1, 2.0f, 3.0, true }));

        const auto arr_test = chowdsp::deserialize_object<std::array<float, 12>> (bytes);
        for (const auto& [x, y] : chowdsp::zip (arr, arr_test))
            REQUIRE (juce::exactlyEqual (x, y));

        const auto str_test = chowdsp::deserialize_string (bytes);
        REQUIRE (str_test == str);
        REQUIRE (bytes.empty());
    }

    SECTION ("Chained Arena w/ previous usage")
    {
        chowdsp::ChainedArenaAllocator arena { 64 };
        arena.allocate<float> (12);
        arena.allocate<float> (12);

        std::vector<float> vec (12);
        std::iota (vec.begin(), vec.end(), 100);
        const auto str = juce::String { "Hello world" };

        const auto frame = arena.create_frame();
        chowdsp::serialize_object (int { 42 }, arena);
        chowdsp::serialize_object (float { 99.0f }, arena);
        chowdsp::serialize_object (Test {}, arena);
        chowdsp::serialize_object (Test { 1, 2.0f, 3.0, true }, arena);
        chowdsp::serialize_span<float> (vec, arena);
        chowdsp::serialize_string (chowdsp::toStringView (str), arena);

        juce::MemoryBlock raw_data {};
        chowdsp::dump_serialized_bytes (raw_data, arena, &frame);
        nonstd::span<const std::byte> bytes { static_cast<std::byte*> (raw_data.getData()), raw_data.getSize() };

        const auto int_test = chowdsp::deserialize_object<int> (bytes);
        REQUIRE (int_test == 42);
        const auto float_test = chowdsp::deserialize_object<float> (bytes);
        REQUIRE (float_test == 99.0f);
        auto struct_test = chowdsp::deserialize_object<Test> (bytes);
        REQUIRE (pfr::eq_fields (struct_test, Test {}));
        struct_test = chowdsp::deserialize_object<Test> (bytes);
        REQUIRE (pfr::eq_fields (struct_test, Test { 1, 2.0f, 3.0, true }));

        std::vector<float> vec_test (chowdsp::get_span_length<float> (bytes));
        REQUIRE (vec_test.size() == vec.size());
        chowdsp::deserialize_span<float> (vec_test, bytes);
        for (const auto& [x, y] : chowdsp::zip (vec, vec_test))
            REQUIRE (juce::exactlyEqual (x, y));

        const auto str_test = chowdsp::deserialize_string (bytes);
        REQUIRE (str_test == chowdsp::toStringView (str));
        REQUIRE (bytes.empty());
    }
}
