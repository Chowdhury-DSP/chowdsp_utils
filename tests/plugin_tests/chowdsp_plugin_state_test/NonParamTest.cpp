#include <CatchUtils.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

TEST_CASE ("Non-Param Test", "[plugin][state]")
{
    SECTION ("JSON Serialization")
    {
        chowdsp::StateValue<int> int_val { "int", 42 };
        chowdsp::StateValue<std::atomic<int>, int> atomic_int_val { "atomic_int", 99 };
        chowdsp::StateValue<std::array<bool, 4>> bool_vals { "bools", { true, false, true, true } };
        chowdsp::StateValue<std::string> string_val { "string", "blah" };
        chowdsp::StateValue<std::string_view> string_view_val { "string_view", "fff" };
        chowdsp::StateValue<juce::String> juce_string_val { "juce_string", "juce" };
        chowdsp::StateValue<chowdsp::json> json_val { "json", { { "val1", 100 }, { "val2", "test" } } };

        chowdsp::json serial;
        {
            chowdsp::NonParamState state {};
            state.addStateValues ({ &int_val, &atomic_int_val, &bool_vals, &string_val, &string_view_val, &juce_string_val, &json_val });
            int_val = 101;
            atomic_int_val.set (102);
            bool_vals.set ({ false, true, false, true });
            string_val = "blah blah";
            string_view_val = "ggg";
            juce_string_val = "ecuj";
            json_val.set ({});
            serial = chowdsp::NonParamState::serialize_json (state);
        }

        chowdsp::NonParamState state {};
        state.addStateValues ({ &int_val, &atomic_int_val, &bool_vals, &string_val, &string_view_val, &juce_string_val, &json_val });
        state.reset();
        REQUIRE (int_val.get() == 42);
        REQUIRE (atomic_int_val.get() == 99);
        REQUIRE (bool_vals.get()[0]);
        REQUIRE (! bool_vals.get()[1]);
        REQUIRE (bool_vals.get()[2]);
        REQUIRE (bool_vals.get()[3]);
        REQUIRE (string_val.get() == "blah");
        REQUIRE (string_view_val.get() == "fff");
        REQUIRE (juce_string_val.get() == "juce");
        REQUIRE (json_val.get() == chowdsp::json { { "val1", 100 }, { "val2", "test" } });

        chowdsp::NonParamState::deserialize_json (serial, state);
        REQUIRE (int_val.get() == 101);
        REQUIRE (atomic_int_val.get() == 102);
        REQUIRE (! bool_vals.get()[0]);
        REQUIRE (bool_vals.get()[1]);
        REQUIRE (! bool_vals.get()[2]);
        REQUIRE (bool_vals.get()[3]);
        REQUIRE (string_val.get() == "blah blah");
        REQUIRE (string_view_val.get() == "ggg");
        REQUIRE (juce_string_val.get() == "ecuj");
        REQUIRE (json_val.get().is_null());
    }

    SECTION ("Bytes Serialization")
    {
        chowdsp::StateValue<int> int_val { "int", 42 };
        chowdsp::StateValue<std::atomic<int>, int> atomic_int_val { "atomic_int", 99 };
        chowdsp::StateValue<std::array<bool, 4>> bool_vals { "bools", { true, false, true, true } };
        chowdsp::StateValue<std::string> string_val { "string", "blah" };
        chowdsp::StateValue<std::string_view> string_view_val { "string_view", "fff" };
        chowdsp::StateValue<juce::String> juce_string_val { "juce_string", "juce" };
        chowdsp::StateValue<chowdsp::json> json_val { "json", { { "val1", 100 }, { "val2", "test" } } };

        chowdsp::ChainedArenaAllocator arena { 1024 };
        {
            chowdsp::NonParamState state {};
            state.addStateValues ({ &int_val, &atomic_int_val, &bool_vals, &string_val, &string_view_val, &juce_string_val, &json_val });
            int_val = 101;
            atomic_int_val.set (102);
            bool_vals.set ({ false, true, false, true });
            string_val = "blah blah";
            string_view_val = "ggg";
            juce_string_val = "ecuj";
            json_val.set ({});
            chowdsp::NonParamState::serialize (arena, state);
        }

        std::vector<std::byte> serial_bytes (chowdsp::get_serial_num_bytes (arena));
        chowdsp::dump_serialized_bytes (serial_bytes, arena);
        nonstd::span<const std::byte> bytes { serial_bytes };

        chowdsp::NonParamState state {};
        state.addStateValues ({ &int_val, &atomic_int_val, &bool_vals, &string_val, &string_view_val, &juce_string_val, &json_val });
        state.reset();
        REQUIRE (int_val.get() == 42);
        REQUIRE (atomic_int_val.get() == 99);
        REQUIRE (bool_vals.get()[0]);
        REQUIRE (! bool_vals.get()[1]);
        REQUIRE (bool_vals.get()[2]);
        REQUIRE (bool_vals.get()[3]);
        REQUIRE (string_val.get() == "blah");
        REQUIRE (string_view_val.get() == "fff");
        REQUIRE (juce_string_val.get() == "juce");
        REQUIRE (json_val.get() == chowdsp::json { { "val1", 100 }, { "val2", "test" } });

        chowdsp::NonParamState::deserialize (bytes, state, arena);
        REQUIRE (int_val.get() == 101);
        REQUIRE (atomic_int_val.get() == 102);
        REQUIRE (! bool_vals.get()[0]);
        REQUIRE (bool_vals.get()[1]);
        REQUIRE (! bool_vals.get()[2]);
        REQUIRE (bool_vals.get()[3]);
        REQUIRE (string_val.get() == "blah blah");
        REQUIRE (string_view_val.get() == "ggg");
        REQUIRE (juce_string_val.get() == "ecuj");
        REQUIRE (json_val.get().is_null());
    }

    SECTION ("Bytes Serialization Re-order")
    {
        chowdsp::StateValue<int> int_val { "int", 42 };
        chowdsp::StateValue<std::atomic<int>, int> atomic_int_val { "atomic_int", 99 };
        chowdsp::StateValue<std::string> string_val { "string", "blah" };

        chowdsp::ChainedArenaAllocator arena { 1024 };
        {
            chowdsp::NonParamState state {};
            state.addStateValues ({ &int_val, &atomic_int_val, &string_val });
            int_val = 101;
            atomic_int_val.set (102);
            string_val = "blah blah";
            chowdsp::NonParamState::serialize (arena, state);
        }

        std::vector<std::byte> serial_bytes (chowdsp::get_serial_num_bytes (arena));
        chowdsp::dump_serialized_bytes (serial_bytes, arena);
        nonstd::span<const std::byte> bytes { serial_bytes };

        chowdsp::NonParamState state {};
        state.addStateValues ({ &string_val, &atomic_int_val, &int_val });
        state.reset();
        REQUIRE (int_val.get() == 42);
        REQUIRE (atomic_int_val.get() == 99);
        REQUIRE (string_val.get() == "blah");

        chowdsp::NonParamState::deserialize (bytes, state, arena);
        REQUIRE (int_val.get() == 101);
        REQUIRE (atomic_int_val.get() == 102);
        REQUIRE (string_val.get() == "blah blah");
    }

    SECTION ("Bytes Serialization Adding Value")
    {
        chowdsp::StateValue<int> int_val { "int", 42 };
        chowdsp::StateValue<std::string> string_val { "string", "blah" };

        chowdsp::ChainedArenaAllocator arena { 1024 };
        {
            chowdsp::NonParamState state {};
            state.addStateValues ({ &int_val, &string_val });
            int_val = 101;
            string_val = "blah blah";
            chowdsp::NonParamState::serialize (arena, state);
        }

        std::vector<std::byte> serial_bytes (chowdsp::get_serial_num_bytes (arena));
        chowdsp::dump_serialized_bytes (serial_bytes, arena);
        nonstd::span<const std::byte> bytes { serial_bytes };

        chowdsp::StateValue<float> float_val { "float", 90.0f };
        chowdsp::NonParamState state {};
        state.addStateValues ({ &string_val, &float_val, &int_val });
        state.reset();
        float_val = 100.0f;
        REQUIRE (int_val.get() == 42);
        REQUIRE (float_val.get() == 100.0f);
        REQUIRE (string_val.get() == "blah");

        chowdsp::NonParamState::deserialize (bytes, state, arena);
        REQUIRE (int_val.get() == 101);
        REQUIRE (float_val.get() == 90.0f);
        REQUIRE (string_val.get() == "blah blah");
    }

    SECTION ("Bytes Serialization Removing Value")
    {
        chowdsp::StateValue<int> int_val { "int", 42 };
        chowdsp::StateValue<std::atomic<int>, int> atomic_int_val { "atomic_int", 99 };
        chowdsp::StateValue<std::string> string_val { "string", "blah" };

        chowdsp::ChainedArenaAllocator arena { 1024 };
        {
            chowdsp::NonParamState state {};
            state.addStateValues ({ &int_val, &atomic_int_val, &string_val });
            int_val = 101;
            atomic_int_val.set (102);
            string_val = "blah blah";
            chowdsp::NonParamState::serialize (arena, state);
        }

        std::vector<std::byte> serial_bytes (chowdsp::get_serial_num_bytes (arena));
        chowdsp::dump_serialized_bytes (serial_bytes, arena);
        nonstd::span<const std::byte> bytes { serial_bytes };

        chowdsp::NonParamState state {};
        state.addStateValues ({ &string_val, &int_val });
        state.reset();
        REQUIRE (int_val.get() == 42);
        REQUIRE (string_val.get() == "blah");

        chowdsp::NonParamState::deserialize (bytes, state, arena);
        REQUIRE (int_val.get() == 101);
        REQUIRE (string_val.get() == "blah blah");
    }

    SECTION ("Bytes Serialization Removing Value")
    {
        chowdsp::StateValue<int> int_val { "int", 42 };
        chowdsp::StateValue<std::atomic<int>, int> atomic_int_val { "atomic_int", 99 };
        chowdsp::StateValue<std::string> string_val { "string", "blah" };

        chowdsp::NonParamState state {};
        state.addStateValues ({ &int_val, &atomic_int_val, &string_val });
        int_val = 101;
        atomic_int_val.set (102);
        string_val = "blah blah";

        const auto serial = std::array<std::byte, 2> {};
        nonstd::span<const std::byte> bytes { serial };

        chowdsp::ChainedArenaAllocator arena { 1024 };
        chowdsp::NonParamState::deserialize (bytes, state, arena);
        REQUIRE (int_val.get() == 42);
        REQUIRE (atomic_int_val.get() == 99);
        REQUIRE (string_val.get() == "blah");
    }
}
