#include <CatchUtils.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

TEST_CASE ("Slider Attachment Test", "[plugin][state][attachments]")
{
    struct Params : chowdsp::ParamHolder
    {
        Params()
        {
            add (param);
        }

        chowdsp::PercentParameter::Ptr param { "pct", "Percent", 0.5f };
    };
    using State = chowdsp::PluginStateImpl<Params>;

    SECTION ("Setup Test")
    {
        State state;
        auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderAttachment attach { param, state, slider };

        REQUIRE_MESSAGE (juce::approximatelyEqual (slider.getValue(), (double) param->get()), "Incorrect slider value after setup!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (slider.getRange().getStart(), 0.0), "Slider range start is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (slider.getRange().getEnd(), 1.0), "Slider range end is incorrect!");

        REQUIRE_MESSAGE (slider.textFromValueFunction (param->get()) == param->getCurrentValueAsText(), "Slider text from value function is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (slider.valueFromTextFunction (param->getCurrentValueAsText()), (double) param->get()), "Slider value from text function is incorrect!");
    }

    SECTION ("Slider Change Test")
    {
        State state;
        const auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderAttachment attach { state.params.param, state, slider };

        static constexpr auto newValue = 0.2f;

        {
            juce::Slider::ScopedDragNotification scopedDrag { slider };
            slider.setValue ((double) newValue, juce::sendNotificationSync);
        }

        REQUIRE_MESSAGE (juce::approximatelyEqual (param->get(), newValue), "Parameter value after slider drag is incorrect!");
    }

    SECTION ("Host Change Test")
    {
        State state;
        auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderAttachment attach { param, state, slider };

        static constexpr auto newValue = 0.7f;

        chowdsp::ParameterTypeHelpers::setValue (newValue, *param);
        state.getParameterListeners().updateBroadcastersFromMessageThread();
        REQUIRE_MESSAGE (juce::approximatelyEqual (slider.getValue(), (double) newValue), "Slider value after parameter change is incorrect!");
    }

    SECTION ("With Undo/Redo Test")
    {
        juce::UndoManager um { 100 };

        State state { &um };
        const auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderAttachment attach { state.params.param, state, slider };

        const auto originalValue = param->get();
        static constexpr auto newValue = 0.99f;

        {
            juce::Slider::ScopedDragNotification scopedDrag { slider };
            slider.setValue ((double) newValue, juce::sendNotificationSync);
        }

        REQUIRE_MESSAGE (juce::approximatelyEqual (param->get(), newValue), "Parameter value after slider drag is incorrect!");
        REQUIRE_MESSAGE (um.canUndo(), "Slider drag is not creating undoable action!");

        um.undo();
        REQUIRE_MESSAGE (juce::approximatelyEqual (param->get(), originalValue), "Parameter value after undo action is incorrect!");
        REQUIRE_MESSAGE (um.canRedo(), "Slider drag undo is not creating redoable action!");

        um.redo();
        REQUIRE_MESSAGE (juce::approximatelyEqual (param->get(), newValue), "Parameter value after redo action is incorrect!");
    }
}

TEST_CASE ("Slider Choice Attachment Test", "[plugin][state][attachments]")
{
    struct Params : chowdsp::ParamHolder
    {
        Params()
        {
            add (param);
        }
        chowdsp::ChoiceParameter::Ptr param { "choice", "Choice", juce::StringArray { "Zero", "One", "Two" }, 1 };
    };
    using State = chowdsp::PluginStateImpl<Params>;

    SECTION ("Setup Test")
    {
        State state;
        auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderChoiceAttachment attach { param, state, slider };

        REQUIRE_MESSAGE (static_cast<int> (slider.getValue()) == param->getIndex(), "Incorrect slider value after setup!");
        REQUIRE_MESSAGE (static_cast<int> (slider.getRange().getStart()) == 0, "Slider range start is incorrect!");
        REQUIRE_MESSAGE (static_cast<int> (slider.getRange().getEnd()) == param->choices.size() - 1, "Slider range end is incorrect!");

        REQUIRE_MESSAGE (slider.textFromValueFunction (param->getIndex()) == param->getCurrentValueAsText(), "Slider text from value function is incorrect!");
        REQUIRE_MESSAGE (static_cast<int> (slider.valueFromTextFunction (param->getCurrentValueAsText())) == param->getIndex(), "Slider value from text function is incorrect!");
    }

    SECTION ("Slider Change Test")
    {
        State state;
        const auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderChoiceAttachment attach { state.params.param, state, slider };

        static constexpr int newValue = 0;

        {
            juce::Slider::ScopedDragNotification scopedDrag { slider };
            slider.setValue ((double) newValue, juce::sendNotificationSync);
        }

        REQUIRE_MESSAGE (param->getIndex() == newValue, "Parameter value after slider drag is incorrect!");
    }

    SECTION ("Host Change Test")
    {
        State state;
        auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderChoiceAttachment attach { param, state, slider };

        static constexpr int newValue = 2;

        chowdsp::ParameterTypeHelpers::setValue (newValue, *param);
        state.getParameterListeners().updateBroadcastersFromMessageThread();
        REQUIRE_MESSAGE (static_cast<int> (slider.getValue()) == newValue, "Slider value after parameter change is incorrect!");
    }

    SECTION ("With Undo/Redo Test")
    {
        juce::UndoManager um { 100 };

        State state { &um };
        const auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderChoiceAttachment attach { state.params.param, state, slider };

        const auto originalValue = param->getIndex();
        static constexpr int newValue = 0;

        {
            juce::Slider::ScopedDragNotification scopedDrag { slider };
            slider.setValue ((double) newValue, juce::sendNotificationSync);
        }

        REQUIRE_MESSAGE (param->getIndex() == newValue, "Parameter value after slider drag is incorrect!");
        REQUIRE_MESSAGE (um.canUndo(), "Slider drag is not creating undoable action!");

        um.undo();
        REQUIRE_MESSAGE (param->getIndex() == originalValue, "Parameter value after undo action is incorrect!");
        REQUIRE_MESSAGE (um.canRedo(), "Slider drag undo is not creating redoable action!");

        um.redo();
        REQUIRE_MESSAGE (param->getIndex() == newValue, "Parameter value after redo action is incorrect!");
    }
}

TEST_CASE ("ComboBox Attachment Test", "[state][attachments]")
{
    struct Params : chowdsp::ParamHolder
    {
        Params()
        {
            add (param);
        }

        chowdsp::ChoiceParameter::Ptr param { "choice", "Choice", juce::StringArray { "One", "Two", "Three" }, 1 };
    };

    using State = chowdsp::PluginStateImpl<Params>;

    SECTION ("Setup Test")
    {
        State state;
        auto& param = state.params.param;

        juce::ComboBox box;
        chowdsp::ComboBoxAttachment attach { param, state, box };

        REQUIRE_MESSAGE (box.getNumItems() == param->choices.size(), "Num choices is incorrect!");
        REQUIRE_MESSAGE (box.getSelectedItemIndex() == param->getIndex(), "Initial choice index is incorrect!");
    }

    SECTION ("Box Change Test")
    {
        State state;
        const auto& param = state.params.param;

        juce::ComboBox box;
        chowdsp::ComboBoxAttachment attach { state.params.param, state, box };

        static constexpr int newValue = 2;
        box.setSelectedItemIndex (newValue, juce::sendNotificationSync);

        REQUIRE_MESSAGE (param->getIndex() == newValue, "Parameter value after box change is incorrect!");
    }

    SECTION ("Host Change Test")
    {
        State state;
        auto& param = state.params.param;

        juce::ComboBox box;
        chowdsp::ComboBoxAttachment attach { param, state, box };

        static constexpr int newValue = 0;

        chowdsp::ParameterTypeHelpers::setValue (newValue, *param);
        state.getParameterListeners().updateBroadcastersFromMessageThread();

        REQUIRE_MESSAGE (box.getSelectedItemIndex() == newValue, "Box value after parameter change is incorrect!");
    }

    SECTION ("With Undo/Redo Test")
    {
        juce::UndoManager um { 100 };

        State state { &um };
        auto& param = state.params.param;

        juce::ComboBox box;
        chowdsp::ComboBoxAttachment attach { param, state, box };

        const auto originalValue = param->getIndex();
        static constexpr int newValue = 2;

        box.setSelectedItemIndex (newValue, juce::sendNotificationSync);

        REQUIRE_MESSAGE (param->getIndex() == newValue, "Parameter value after box change is incorrect!");
        REQUIRE_MESSAGE (um.canUndo(), "Slider drag is not creating undoable action!");

        um.undo();
        REQUIRE_MESSAGE (param->getIndex() == originalValue, "Parameter value after undo action is incorrect!");
        REQUIRE_MESSAGE (um.canRedo(), "Slider drag undo is not creating redoable action!");

        um.redo();
        REQUIRE_MESSAGE (param->getIndex() == newValue, "Parameter value after redo action is incorrect!");
    }
}

TEST_CASE ("Button Attachment Test", "[state][attachments]")
{
    struct Params : chowdsp::ParamHolder
    {
        Params()
        {
            add (param);
        }

        chowdsp::BoolParameter::Ptr param { "bool", "Bool", false };
    };

    using State = chowdsp::PluginStateImpl<Params>;

    SECTION ("Setup Test")
    {
        State state;
        auto& param = state.params.param;

        juce::ToggleButton button;
        chowdsp::ButtonAttachment attach { param, state, button };

        REQUIRE_MESSAGE (button.getButtonText() == param->name, "Button name is incorrect!");
        REQUIRE_MESSAGE (button.getToggleState() == param->get(), "Initial button state is incorrect!");
    }

    SECTION ("Button Change Test")
    {
        State state;
        const auto& param = state.params.param;

        juce::ToggleButton button;
        chowdsp::ButtonAttachment attach { state.params.param, state, button };

        static constexpr bool newValue = true;
        button.setToggleState (newValue, juce::sendNotificationSync);

        REQUIRE_MESSAGE (param->get() == newValue, "Parameter value after button change is incorrect!");
    }

    SECTION ("Host Change Test")
    {
        State state;
        auto& param = state.params.param;

        juce::ToggleButton button;
        chowdsp::ButtonAttachment attach { param, state, button };

        static constexpr bool newValue = true;

        chowdsp::ParameterTypeHelpers::setValue (newValue, *param);
        state.getParameterListeners().updateBroadcastersFromMessageThread();

        REQUIRE_MESSAGE (button.getToggleState() == newValue, "Button state after parameter change is incorrect!");
    }

    SECTION ("With Undo/Redo Test")
    {
        juce::UndoManager um { 100 };

        State state { &um };
        auto& param = state.params.param;

        juce::ToggleButton button;
        chowdsp::ButtonAttachment attach { param, state, button };

        const auto originalValue = param->get();
        static constexpr bool newValue = true;

        button.setToggleState (newValue, juce::sendNotificationSync);

        REQUIRE_MESSAGE (param->get() == newValue, "Parameter value after button change is incorrect!");
        REQUIRE_MESSAGE (um.canUndo(), "Slider drag is not creating undoable action!");

        um.undo();
        REQUIRE_MESSAGE (param->get() == originalValue, "Parameter value after undo action is incorrect!");
        REQUIRE_MESSAGE (um.canRedo(), "Slider drag undo is not creating redoable action!");

        um.redo();
        REQUIRE_MESSAGE (param->get() == newValue, "Parameter value after redo action is incorrect!");
    }
}

TEST_CASE ("Custom Parameter Attachment Test", "[state][attachments]")
{
    struct Params : chowdsp::ParamHolder
    {
        Params()
        {
            add (param);
        }

        chowdsp::BoolParameter::Ptr param { "bool", "Bool", true };
    };

    using State = chowdsp::PluginStateImpl<Params>;

    State state;
    auto& param = state.params.param;

    juce::Component comp;
    chowdsp::ParameterAttachment<chowdsp::BoolParameter> attach { param, state, [&comp] (bool val)
                                                                  { comp.setVisible (val); } };

    REQUIRE (! comp.isVisible());

    attach.manuallyTriggerUpdate();
    REQUIRE (comp.isVisible());

    chowdsp::ParameterTypeHelpers::setValue (false, *param);
    state.getParameterListeners().updateBroadcastersFromMessageThread();
    REQUIRE (! comp.isVisible());
}
