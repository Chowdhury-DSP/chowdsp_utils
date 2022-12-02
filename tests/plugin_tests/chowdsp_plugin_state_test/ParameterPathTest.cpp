#include <TimedUnitTest.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

class ParameterPathTest : public TimedUnitTest
{
public:
    ParameterPathTest() : TimedUnitTest ("Parameter Path Test", "ChowDSP State") {}

    static void pathSplitTest()
    {
        using namespace std::string_view_literals;
        static constexpr chowdsp::ParameterPath path { "group1/group2/param"sv };
        static_assert (path.head == "group1"sv);

        static constexpr chowdsp::ParameterPath path2 { path.tail };
        static_assert (path2.head == "group2"sv);

        static constexpr chowdsp::ParameterPath param { path2.tail };
        static_assert (param.head == "param"sv);
        static_assert (param.tail.empty());
    }

    struct Group1
    {
        chowdsp::PercentParameter::Ptr percent { "percent", "Percent", 0.5f };
    };

    struct Group2
    {
        Group1 group1;
        chowdsp::ChoiceParameter::Ptr choice { "choice", "Choice", juce::StringArray { "One", "Two", "Three" }, 1 };
    };

    struct Params
    {
        Group2 group2;
        chowdsp::BoolParameter::Ptr boolean { "bool", "Bool", false };
    };

    void paramAccessTest()
    {
        using State = chowdsp::PluginState<Params>;
        State state;
        auto& constState = const_cast<const State&> (state);

        using namespace std::string_view_literals;
        expect (state.params.boolean.get() == &state.getParameter<chowdsp::BoolParameter> ("bool"sv), "Unable to access top-level parameter!");
        expect (state.params.group2.choice.get() == &state.getParameter<chowdsp::ChoiceParameter> ("Group2/choice"sv), "Unable to access nested parameter!");
        expect (state.params.group2.group1.percent.get() == &constState.getParameter<chowdsp::PercentParameter> ("Group2/Group1/percent"sv), "Unable to access doubly-nested parameter!");
    }

    void parameterAttachmentTest()
    {
        using State = chowdsp::PluginState<Params>;
        State state;

        chowdsp::ParameterAttachment<chowdsp::BoolParameter, State> attachment { std::string_view { "bool" }, state, [] (bool) {} };
        expect (state.params.boolean.get() == &attachment.param, "Unable to create attachment from parameter path!");
    }

    void runTestTimed() override
    {
        beginTest ("Path Split Test");
        pathSplitTest();

        beginTest ("Param Access Test");
        paramAccessTest();

        beginTest ("Param Attachment Test");
        parameterAttachmentTest();
    }
};

static ParameterPathTest parameterPathTest;
