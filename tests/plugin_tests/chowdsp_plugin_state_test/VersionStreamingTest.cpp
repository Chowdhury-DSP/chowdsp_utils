#include <CatchUtils.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

using namespace chowdsp::version_literals;

struct Params : chowdsp::ParamHolder
{
    Params()
    {
        add (param, innerParams);
    }

    chowdsp::BoolParameter::Ptr param { "bool", "Bool", false };

    struct InnerParams : chowdsp::ParamHolder
    {
        InnerParams()
        {
            add (param);
            versionStreamingCallback = [this] (const chowdsp::Version& version)
            {
                if (version >= "1.0.0"_v)
                    param->setValueNotifyingHost (1.0f);
            };
        }

        chowdsp::BoolParameter::Ptr param { "bool2", "Bool", false };
    } innerParams;
};

struct NonParams : chowdsp::NonParamState
{
    NonParams()
    {
        addStateValues ({ &editorSize });
        versionStreamingCallback = [this] (const chowdsp::Version& version)
        {
            if (version <= "9.9.9"_v)
                editorSize.set (1.5f);
        };
    }

    chowdsp::StateValue<float> editorSize { "editor_size", 1.0f };
};

TEST_CASE ("Version Streaming Test", "[plugin][state][version]")
{
    SECTION ("Apply Version Streaming to Parameters")
    {
        {
            Params params;
            REQUIRE (params.innerParams.param->get() == false);
            params.applyVersionStreaming ("0.9.9"_v);
            REQUIRE (params.innerParams.param->get() == false);
        }

        {
            Params params;
            REQUIRE (params.innerParams.param->get() == false);
            params.applyVersionStreaming ("1.0.1"_v);
            REQUIRE (params.innerParams.param->get() == true);
        }
    }

    SECTION ("Apply Version Streaming to Non-Parameters")
    {
        NonParams nonParams;
        REQUIRE (juce::approximatelyEqual (nonParams.editorSize.get(), 1.0f));
        nonParams.versionStreamingCallback ("1.0.0"_v);
        REQUIRE (juce::approximatelyEqual (nonParams.editorSize.get(), 1.5f));
    }

    SECTION ("Version Streaming with State Serialization")
    {
        static_assert (chowdsp::currentPluginVersion == "9.9.9"_v, "Tests are tuned for JucePlugin_VersionString = 9.9.9");

        using State = chowdsp::PluginStateImpl<Params, NonParams>;

        juce::MemoryBlock stateBlock;

        {
            // Save initial state
            State state {};
            state.serialize (stateBlock);
        }

        // check new state
        State state {};
        REQUIRE (state.params.innerParams.param->get() == false);
        REQUIRE (juce::approximatelyEqual (state.nonParams.editorSize.get(), 1.0f));
        state.deserialize (stateBlock);
        REQUIRE (state.params.innerParams.param->get() == true);
        REQUIRE (juce::approximatelyEqual (state.nonParams.editorSize.get(), 1.5f));
    }
}
