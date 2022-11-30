#include <TimedUnitTest.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

using namespace std::string_view_literals;

class SliderAttachmentTest : public TimedUnitTest
{
public:
    SliderAttachmentTest() : TimedUnitTest ("Slider Attachment Test", "ChowDSP State")
    {
    }

    struct Params
    {
        chowdsp::PercentParameter::Ptr param { "pct", "Percent", 0.5f };
    };

    using State = chowdsp::PluginState<Params>;

    void setupTest()
    {
        State state;
        auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderAttachment attach { "pct"sv, state, slider };

        expectEquals (slider.getValue(), (double) param->get(), "Incorrect slider value after setup!");
        expectEquals (slider.getRange().getStart(), 0.0, "Slider range start is incorrect!");
        expectEquals (slider.getRange().getEnd(), 1.0, "Slider range end is incorrect!");

        expectEquals (slider.textFromValueFunction (param->get()), param->getCurrentValueAsText(), "Slider text from value function is incorrect!");
        expectEquals (slider.valueFromTextFunction (param->getCurrentValueAsText()), (double) param->get(), "Slider value from text function is incorrect!");
    }

    void sliderParamChangeTest()
    {
        State state;
        const auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderAttachment attach { "pct"sv, state, slider };

        static constexpr auto newValue = 0.2f;

        {
            juce::Slider::ScopedDragNotification scopedDrag { slider };
            slider.setValue ((double) newValue, juce::sendNotificationSync);
        }

        expectEquals (param->get(), newValue, "Parameter value after slider drag is incorrect!");
    }

    void hostParamChangeTest()
    {
        State state;
        auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderAttachment attach { "pct"sv, state, slider };

        static constexpr auto newValue = 0.7f;

        chowdsp::ParameterTypeHelpers::setValue (newValue, *param);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (20);

        expectEquals (slider.getValue(), (double) newValue, "Slider value after parameter change is incorrect!");
    }

    void withUndoRedo()
    {
        juce::UndoManager um { 100 };

        State state { &um };
        const auto& param = state.params.param;

        juce::Slider slider;
        chowdsp::SliderAttachment attach { "pct"sv, state, slider };

        const auto originalValue = param->get();
        static constexpr auto newValue = 0.99f;

        {
            juce::Slider::ScopedDragNotification scopedDrag { slider };
            slider.setValue ((double) newValue, juce::sendNotificationSync);
        }

        expectEquals (param->get(), newValue, "Parameter value after slider drag is incorrect!");
        expect (um.canUndo(), "Slider drag is not creating undoable action!");

        um.undo();
        expectEquals (param->get(), originalValue, "Parameter value after undo action is incorrect!");
        expect (um.canRedo(), "Slider drag undo is not creating redoable action!");

        um.redo();
        expectEquals (param->get(), newValue, "Parameter value after redo action is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Setup Test");
        setupTest();

        beginTest ("Slider Change Test");
        sliderParamChangeTest();

        beginTest ("Host Change Test");
        hostParamChangeTest();

        beginTest ("With Undo/Redo Test");
        withUndoRedo();
    }
};

class ComboBoxAttachmentTest : public TimedUnitTest
{
public:
    ComboBoxAttachmentTest() : TimedUnitTest ("ComboBox Attachment Test", "ChowDSP State")
    {
    }

    struct Params
    {
        chowdsp::ChoiceParameter::Ptr param { "choice", "Choice", juce::StringArray { "One", "Two", "Three" }, 1 };
    };

    using State = chowdsp::PluginState<Params>;

    void setupTest()
    {
        State state;
        auto& param = state.params.param;

        juce::ComboBox box;
        chowdsp::ComboBoxAttachment attach { "choice"sv, state, box };

        expectEquals (box.getNumItems(), param->choices.size(), "Num choices is incorrect!");
        expectEquals (box.getSelectedItemIndex(), param->getIndex(), "Initial choice index is incorrect!");
    }

    void boxParamChangeTest()
    {
        State state;
        const auto& param = state.params.param;

        juce::ComboBox box;
        chowdsp::ComboBoxAttachment attach { "choice"sv, state, box };

        static constexpr int newValue = 2;
        box.setSelectedItemIndex (newValue, juce::sendNotificationSync);

        expectEquals (param->getIndex(), newValue, "Parameter value after box change is incorrect!");
    }

    void hostParamChangeTest()
    {
        State state;
        auto& param = state.params.param;

        juce::ComboBox box;
        chowdsp::ComboBoxAttachment attach { "choice"sv, state, box };

        static constexpr int newValue = 0;

        chowdsp::ParameterTypeHelpers::setValue (newValue, *param);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (20);

        expectEquals (box.getSelectedItemIndex(), newValue, "Box value after parameter change is incorrect!");
    }

    void withUndoRedo()
    {
        juce::UndoManager um { 100 };

        State state { &um };
        auto& param = state.params.param;

        juce::ComboBox box;
        chowdsp::ComboBoxAttachment attach { "choice"sv, state, box };

        const auto originalValue = param->getIndex();
        static constexpr int newValue = 2;

        box.setSelectedItemIndex (newValue, juce::sendNotificationSync);

        expectEquals (param->getIndex(), newValue, "Parameter value after box change is incorrect!");
        expect (um.canUndo(), "Slider drag is not creating undoable action!");

        um.undo();
        expectEquals (param->getIndex(), originalValue, "Parameter value after undo action is incorrect!");
        expect (um.canRedo(), "Slider drag undo is not creating redoable action!");

        um.redo();
        expectEquals (param->getIndex(), newValue, "Parameter value after redo action is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Setup Test");
        setupTest();

        beginTest ("Box Change Test");
        boxParamChangeTest();

        beginTest ("Host Change Test");
        hostParamChangeTest();

        beginTest ("With Undo/Redo Test");
        withUndoRedo();
    }
};

class ButtonAttachmentTest : public TimedUnitTest
{
public:
    ButtonAttachmentTest() : TimedUnitTest ("Button Attachment Test", "ChowDSP State")
    {
    }

    struct Params
    {
        chowdsp::BoolParameter::Ptr param { "bool", "Bool", false };
    };

    using State = chowdsp::PluginState<Params>;

    void setupTest()
    {
        State state;
        auto& param = state.params.param;

        juce::ToggleButton button;
        chowdsp::ButtonAttachment attach { "bool"sv, state, button };

        expectEquals (button.getButtonText(), param->name, "Button name is incorrect!");
        expect (button.getToggleState() == param->get(), "Initial button state is incorrect!");
    }

    void boxParamChangeTest()
    {
        State state;
        const auto& param = state.params.param;

        juce::ToggleButton button;
        chowdsp::ButtonAttachment attach { "bool"sv, state, button };

        static constexpr bool newValue = true;
        button.setToggleState (newValue, juce::sendNotificationSync);

        expect (param->get() == newValue, "Parameter value after button change is incorrect!");
    }

    void hostParamChangeTest()
    {
        State state;
        auto& param = state.params.param;

        juce::ToggleButton button;
        chowdsp::ButtonAttachment attach { "bool"sv, state, button };

        static constexpr bool newValue = true;

        chowdsp::ParameterTypeHelpers::setValue (newValue, *param);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (20);

        expect (button.getToggleState() == newValue, "Button state after parameter change is incorrect!");
    }

    void withUndoRedo()
    {
        juce::UndoManager um { 100 };

        State state { &um };
        auto& param = state.params.param;

        juce::ToggleButton button;
        chowdsp::ButtonAttachment attach { "bool"sv, state, button };

        const auto originalValue = param->get();
        static constexpr bool newValue = true;

        button.setToggleState (newValue, juce::sendNotificationSync);

        expect (param->get() == newValue, "Parameter value after button change is incorrect!");
        expect (um.canUndo(), "Slider drag is not creating undoable action!");

        um.undo();
        expect (param->get() == originalValue, "Parameter value after undo action is incorrect!");
        expect (um.canRedo(), "Slider drag undo is not creating redoable action!");

        um.redo();
        expect (param->get() == newValue, "Parameter value after redo action is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Setup Test");
        setupTest();

        beginTest ("Box Change Test");
        boxParamChangeTest();

        beginTest ("Host Change Test");
        hostParamChangeTest();

        beginTest ("With Undo/Redo Test");
        withUndoRedo();
    }
};

static SliderAttachmentTest sliderAttachmentTest;
static ComboBoxAttachmentTest comboBoxAttachmentTest;
static ButtonAttachmentTest buttonAttachmentTest;
