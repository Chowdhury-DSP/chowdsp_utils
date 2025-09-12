#include <CatchUtils.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

struct AggregateType
{
    juce::String a;
    juce::String b;

    friend bool operator== (const AggregateType& lhs, const AggregateType& rhs) { return lhs.a == rhs.a
                                                                                         && lhs.b == rhs.b; }
    friend bool operator!= (const AggregateType& lhs, const AggregateType& rhs) { return ! (lhs == rhs); }
};

template <>
struct nlohmann::adl_serializer<AggregateType>
{
    static void to_json (json& j, const AggregateType& a)
    {
        j = { { "a_value", a.a }, { "b_value", a.b } };
    }

    static void from_json (const json& j, AggregateType& a)
    {
        a.a = j.value ("a_value", juce::String ("default string"));
        a.b = j.value ("b_value", juce::String ("default string"));
    }
};

TEST_CASE ("Non-Param Test", "[plugin][state][serial]")
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
        chowdsp::StateValue<AggregateType> aggregate_val { "aggregate", { "thing1", "thing2" } };

        chowdsp::json serial;
        {
            chowdsp::NonParamState state {};
            state.addStateValues ({
                &int_val,
                &atomic_int_val,
                &bool_vals,
                &string_val,
                &string_view_val,
                &juce_string_val,
                &json_val,
                &aggregate_val,
            });
            int_val = 101;
            atomic_int_val.set (102);
            bool_vals.set ({ false, true, false, true });
            string_val = "blah blah";
            string_view_val = "ggg";
            juce_string_val = "ecuj";
            json_val.set ({});
            aggregate_val.set ({ "blah1", "blah2" });
            serial = chowdsp::NonParamState::serialize_json (state);
        }

        chowdsp::NonParamState state {};
        state.addStateValues ({
            &int_val,
            &atomic_int_val,
            &bool_vals,
            &string_val,
            &string_view_val,
            &juce_string_val,
            &json_val,
            &aggregate_val,
        });
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
        REQUIRE (aggregate_val.get() == AggregateType { "thing1", "thing2" });

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
        REQUIRE (aggregate_val.get() == AggregateType { "blah1", "blah2" });
    }
}
